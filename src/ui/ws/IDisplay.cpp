/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 12 дек. 2016 г.
 *
 * lsp-plugins is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-plugins is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-plugins. If not, see <https://www.gnu.org/licenses/>.
 */

#include <core/status.h>
#include <ui/ws/ws.h>
#include <core/io/Dir.h>
#include <metadata/metadata.h>

#ifdef LSP_IDE_DEBUG
    #ifdef PLATFORM_UNIX_COMPATIBLE
        #include <rendering/glx/factory.h>

        namespace lsp
        {
            extern glx_factory_t   glx_factory;
        }
    #endif
#endif /* LSP_IDE_DEBUG */

namespace lsp
{
    namespace ws
    {
        IDisplay::IDisplay()
        {
            nTaskID             = 0;
            s3DFactory          = NULL;
            nCurrent3D          = 0;
            nPending3D          = 0;
            sMainTask.nID       = 0;
            sMainTask.nTime     = 0;
            sMainTask.pHandler  = NULL;
            sMainTask.pArg      = NULL;
        }

        IDisplay::~IDisplay()
        {
        }

        const R3DBackendInfo *IDisplay::enum_backend(size_t id) const
        {
            return s3DLibs.get(id);
        };

        const R3DBackendInfo *IDisplay::current_backend() const
        {
            return s3DLibs.get(nCurrent3D);
        }

        ssize_t IDisplay::current_backend_id() const
        {
            return nCurrent3D;
        }

        status_t IDisplay::select_backend(const R3DBackendInfo *backend)
        {
            if (backend == NULL)
                return STATUS_BAD_ARGUMENTS;

            const r3d_library_t *lib = static_cast<const r3d_library_t *>(backend);
            ssize_t index   = s3DLibs.index_of(lib);
            if (index < 0)
                return STATUS_NOT_FOUND;

            nPending3D      = index;

            return STATUS_OK;
        }

        status_t IDisplay::select_backend_id(size_t id)
        {
            const r3d_library_t *lib = s3DLibs.get(id);
            if (lib == NULL)
                return STATUS_NOT_FOUND;

            nPending3D      = id;
            return STATUS_OK;
        }

        void IDisplay::lookup_r3d_backends(const io::Path *path)
        {
            io::Dir dir;

            status_t res = dir.open(path);
            if (res != STATUS_OK)
                return;

            io::Path child;
            LSPString item, prefix, postfix;
            if (!prefix.set_ascii(LSP_R3D_BACKEND_PREFIX))
                return;

            io::fattr_t fattr;
            while ((res = dir.read(&item, false)) == STATUS_OK)
            {
                if (!item.starts_with(&prefix))
                    continue;

                if ((res = child.set(path, &item)) != STATUS_OK)
                    continue;
                if ((res = child.stat(&fattr)) != STATUS_OK)
                    continue;

                switch (fattr.type)
                {
                    case io::fattr_t::FT_DIRECTORY:
                    case io::fattr_t::FT_BLOCK:
                    case io::fattr_t::FT_CHARACTER:
                        continue;
                    default:
                        register_r3d_backend(&child);
                        break;
                }
            }
        }

        void IDisplay::lookup_r3d_backends(const char *path)
        {
            io::Path tmp;
            if (tmp.set(path) != STATUS_OK)
                return;
            lookup_r3d_backends(&tmp);
        }

        void IDisplay::lookup_r3d_backends(const LSPString *path)
        {
            io::Path tmp;
            if (tmp.set(path) != STATUS_OK)
                return;
            lookup_r3d_backends(&tmp);
        }

        status_t IDisplay::register_r3d_backend(const io::Path *path)
        {
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            return register_r3d_backend(path->as_string());
        }

        status_t IDisplay::register_r3d_backend(const char *path)
        {
            LSPString tmp;
            if (path == NULL)
                return STATUS_BAD_ARGUMENTS;
            if (!tmp.set_utf8(path))
                return STATUS_NO_MEM;
            return register_r3d_backend(&tmp);
        }

        status_t IDisplay::commit_r3d_factory(const LSPString *path, r3d_factory_t *factory)
        {
            for (size_t id=0; ; ++id)
            {
                // Get metadata record
                const r3d_backend_metadata_t *meta = factory->metadata(factory, id);
                if (meta == NULL)
                    break;
                else if (meta->id == NULL)
                    continue;

                // Create library descriptor
                r3d_library_t *r3dlib   = new r3d_library_t();
                if (r3dlib == NULL)
                    return STATUS_NO_MEM;

                r3dlib->builtin     = (path != NULL) ? NULL : factory;
                r3dlib->local_id    = id;
                if (path != NULL)
                {
                    if (!r3dlib->library.set(path))
                    {
                        delete r3dlib;
                        return STATUS_NO_MEM;
                    }
                }

                if ((!r3dlib->uid.set_utf8(meta->id)) ||
                    (!r3dlib->display.set_utf8((meta->display != NULL) ? meta->display : meta->id)))
                {
                    delete r3dlib;
                    return STATUS_NO_MEM;
                }

                // Add library descriptor to the list
                if (!s3DLibs.add(r3dlib))
                {
                    delete r3dlib;
                    return STATUS_NO_MEM;
                }
            }

            return STATUS_OK;
        }

        status_t IDisplay::register_r3d_backend(const LSPString *path)
        {
            ipc::Library lib;

            // Open library
            status_t res = lib.open(path);
            if (res != STATUS_OK)
                return res;

            // Lookup function
            lsp_r3d_factory_function_t func = reinterpret_cast<lsp_r3d_factory_function_t>(lib.import(R3D_FACTORY_FUNCTION_NAME));
            if (func == NULL)
            {
                lib.close();
                return STATUS_NOT_FOUND;
            }

            // Try to instantiate factory
            r3d_factory_t *factory  = func(LSP_MAIN_VERSION);
            if (factory == NULL)
            {
                lib.close();
                return STATUS_NOT_FOUND;
            }

            // Fetch metadata
            res = commit_r3d_factory(path, factory);

            // Close the library
            lib.close();
            return res;
        }

        int IDisplay::init(int argc, const char **argv)
        {
            status_t res;

            #ifdef LSP_IDE_DEBUG
                #ifdef PLATFORM_UNIX_COMPATIBLE
                    res = commit_r3d_factory(NULL, &glx_factory); // Remember built-in factory
                    if (res != STATUS_OK)
                        return res;
                #endif
            #endif /* LSP_IDE_DEBUG */

            // Scan for another locations
            io::Path path;
            res = ipc::Library::get_self_file(&path);
            if (res == STATUS_OK)
                res     = path.parent();
            if (res == STATUS_OK)
                lookup_r3d_backends(&path);

            return STATUS_OK;
        }

        void IDisplay::destroy()
        {
            // Destroy all backends
            for (size_t j=0,m=s3DBackends.size(); j<m;++j)
            {
                // Get backend
                IR3DBackend *backend = s3DBackends.get(j);
                if (backend == NULL)
                    continue;

                // Destroy backend
                backend->destroy();
                delete backend;
            }

            // Destroy all libs
            for (size_t j=0, m=s3DLibs.size(); j<m; ++j)
            {
                r3d_library_t *r3dlib = s3DLibs.at(j);
                delete r3dlib;
            }

            // Flush list of backends and close library
            s3DLibs.flush();
            s3DBackends.flush();
            s3DFactory = NULL;
            s3DLibrary.close();
        }

        void IDisplay::detach_r3d_backends()
        {
            // Destroy all backends
            for (size_t j=0,m=s3DBackends.size(); j<m;++j)
            {
                // Get backend
                IR3DBackend *backend = s3DBackends.get(j);
                if (backend != NULL)
                    backend->destroy();
            }
        }

        void IDisplay::call_main_task(timestamp_t time)
        {
            if (sMainTask.pHandler != NULL)
                sMainTask.pHandler(time, sMainTask.pArg);
        }

        int IDisplay::main()
        {
            return STATUS_SUCCESS;
        }

        void IDisplay::sync()
        {
        }

        void IDisplay::deregister_backend(IR3DBackend *backend)
        {
            // Try to remove backend
            if (!s3DBackends.remove(backend, true))
                return;

            // Need to unload library?
            if (s3DBackends.size() <= 0)
            {
                s3DFactory  = NULL;
                s3DLibrary.close();
            }
        }

        IR3DBackend *IDisplay::create_r3D_backend(INativeWindow *parent)
        {
            if (parent == NULL)
                return NULL;

            // Obtain current backend
            r3d_library_t *lib = s3DLibs.get(nCurrent3D);
            if (lib == NULL)
                return NULL;

            // Check that factory is present
            if (s3DFactory == NULL)
            {
                if (s3DBackends.size() > 0)
                    return NULL;

                // Try to load factory
                if (switch_r3d_backend(lib) != STATUS_OK)
                    return NULL;
            }

            // Call factory to create backend
            r3d_backend_t *backend = s3DFactory->create(s3DFactory, lib->local_id);
            if (backend == NULL)
                return NULL;

            // Initialize backend
            void *handle = NULL;
            status_t res = backend->init_offscreen(backend);
            if (res != STATUS_OK)
            {
                res = backend->init_window(backend, &handle);
                if (res != STATUS_OK)
                {
                    backend->destroy(backend);
                    return NULL;
                }
            }

            // Create R3D backend wrapper
            IR3DBackend *r3d = new IR3DBackend(this, backend, parent->handle(), handle);
            if (r3d == NULL)
            {
                backend->destroy(backend);
                return NULL;
            }

            // Register backend wrapper
            if (!s3DBackends.add(r3d))
            {
                r3d->destroy();
                delete r3d;
                return NULL;
            }

            // Return successful operation
            return r3d;
        }

        status_t IDisplay::switch_r3d_backend(r3d_library_t *lib)
        {
            status_t res;
            r3d_factory_t *factory;
            ipc::Library dlib;

            if (!lib->builtin)
            {
                // Load the library
                res = dlib.open(&lib->library);
                if (res != STATUS_OK)
                    return res;

                // Obtain factory function
                lsp_r3d_factory_function_t func = reinterpret_cast<lsp_r3d_factory_function_t>(dlib.import(R3D_FACTORY_FUNCTION_NAME));
                if (func == NULL)
                {
                    dlib.close();
                    return res;
                }

                // Create the factory
                factory     = func(LSP_MAIN_VERSION);
                if (factory == NULL)
                {
                    dlib.close();
                    return STATUS_UNKNOWN_ERR;
                }
            }
            else
                factory     = lib->builtin;

            // Now, iterate all registered backend wrappers and change the backend
            for (size_t i=0, n=s3DBackends.size(); i<n; ++i)
            {
                // Obtain current backend
                IR3DBackend *r3d = s3DBackends.get(i);
                if (r3d == NULL)
                    continue;

                // Call factory to create backend
                void *handle = NULL;
                r3d_backend_t *backend = factory->create(factory, lib->local_id);
                if (backend != NULL)
                {
                    // Initialize backend
                    status_t res = backend->init_offscreen(backend);
                    if (res != STATUS_OK)
                    {
                        res = backend->init_window(backend, &handle);
                        if (res != STATUS_OK)
                        {
                            backend->destroy(backend);
                            backend = NULL;
                            handle  = NULL;
                        }
                    }
                }

                // Call backend wrapper to replace the backend
                r3d->replace_backend(backend, handle);
            }

            // Deregister currently used library
            dlib.swap(&s3DLibrary);
            dlib.close();

            // Register new pointer to the backend factory
            s3DFactory  = factory;

            return STATUS_OK;
        }

        status_t IDisplay::main_iteration()
        {
            // Sync backends
            if (nCurrent3D != nPending3D)
            {
                r3d_library_t *lib = s3DLibs.get(nPending3D);
                if (lib != NULL)
                {
                    if (switch_r3d_backend(lib) == STATUS_OK)
                        nCurrent3D = nPending3D;
                }
                else
                    nPending3D = nCurrent3D;
            }

            return STATUS_SUCCESS;
        }

        void IDisplay::quit_main()
        {
        }

        size_t IDisplay::screens()
        {
            return 0;
        }

        size_t IDisplay::default_screen()
        {
            return 0;
        }

        status_t IDisplay::screen_size(size_t screen, ssize_t *w, ssize_t *h)
        {
            return STATUS_BAD_ARGUMENTS;
        }

        INativeWindow *IDisplay::create_window()
        {
            return NULL;
        }

        INativeWindow *IDisplay::create_window(size_t screen)
        {
            return NULL;
        }

        INativeWindow *IDisplay::create_window(void *handle)
        {
            return NULL;
        }

        INativeWindow *IDisplay::wrap_window(void *handle)
        {
            return NULL;
        }

        ISurface *IDisplay::create_surface(size_t width, size_t height)
        {
            return NULL;
        }
    
        bool IDisplay::taskid_exists(taskid_t id)
        {
            for (size_t i=0, n=sTasks.size(); i<n; ++i)
            {
                dtask_t *task = sTasks.at(i);
                if (task == NULL)
                    continue;
                if (task->nID == id)
                    return true;
            }
            return false;
        }

        taskid_t IDisplay::submit_task(timestamp_t time, task_handler_t handler, void *arg)
        {
            if (handler == NULL)
                return -STATUS_BAD_ARGUMENTS;

            ssize_t first = 0, last = sTasks.size() - 1;

            // Find the place to add the task
            while (first <= last)
            {
                ssize_t center  = (first + last) >> 1;
                dtask_t *t      = sTasks.at(center);
                if (t->nTime <= time)
                    first           = center + 1;
                else
                    last            = center - 1;
            }

            // Generate task ID
            do
            {
                nTaskID     = (nTaskID + 1) & 0x7fffff;
            } while (taskid_exists(nTaskID));

            // Add task to the found place keeping it's time order
            dtask_t *t = sTasks.insert(first);
            if (t == NULL)
                return -STATUS_NO_MEM;

            t->nID          = nTaskID;
            t->nTime        = time;
            t->pHandler     = handler;
            t->pArg         = arg;

            return t->nID;
        }

        status_t IDisplay::cancel_task(taskid_t id)
        {
            if (id < 0)
                return STATUS_INVALID_UID;

            // Remove task from the queue
            for (size_t i=0; i<sTasks.size(); ++i)
                if (sTasks.at(i)->nID == id)
                {
                    sTasks.remove(i);
                    return STATUS_OK;
                }

            return STATUS_NOT_FOUND;
        }

        status_t IDisplay::set_clipboard(size_t id, IDataSource *c)
        {
            if (c == NULL)
                return STATUS_BAD_ARGUMENTS;
            c->acquire();
            c->release();
            return STATUS_NOT_IMPLEMENTED;
        }

        status_t IDisplay::get_clipboard(size_t id, IDataSink *dst)
        {
            if (dst == NULL)
                return STATUS_BAD_ARGUMENTS;
            dst->acquire();
            dst->release();
            return STATUS_NOT_IMPLEMENTED;
        }

        status_t IDisplay::reject_drag()
        {
            return STATUS_NOT_IMPLEMENTED;
        }

        status_t IDisplay::accept_drag(IDataSink *sink, drag_t action, bool internal, const realize_t *r)
        {
            return STATUS_NOT_IMPLEMENTED;
        }

        const char * const *IDisplay::get_drag_ctypes()
        {
            return NULL;
        }

        void IDisplay::set_main_callback(task_handler_t handler, void *arg)
        {
            sMainTask.pHandler      = handler;
            sMainTask.pArg          = arg;
        }
    }

} /* namespace lsp */
