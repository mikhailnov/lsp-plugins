/*
 * Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins
 * Created on: 26 февр. 2020 г.
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

#ifndef CORE_I18N_ROOTDICTIONARY_H_
#define CORE_I18N_ROOTDICTIONARY_H_

#include <data/cvector.h>
#include <core/i18n/IDictionary.h>
#include <core/io/Path.h>

namespace lsp
{
    /**
     * Class implements root dictionary logic which operates on all
     * sub-dictionaries
     */
    class Dictionary: public IDictionary
    {
        private:
            Dictionary & operator = (const Dictionary &);

        protected:
            typedef struct node_t
            {
                LSPString       sKey;
                IDictionary    *pDict;
                bool            bRoot;
            } node_t;

        protected:
            cvector<node_t>     vNodes;
            LSPString           sPath;

        protected:
            status_t        load_json(IDictionary **dict, const LSPString *path);
            status_t        load_builtin(IDictionary **dict, const LSPString *path);
            status_t        create_child(IDictionary **dict, const LSPString *path);
            status_t        init_dictionary(IDictionary *d, const LSPString *path);
            status_t        load_dictionary(const LSPString *id, IDictionary **dict);

        public:
            explicit Dictionary();
            virtual ~Dictionary();

        public:
            using IDictionary::lookup;
            using IDictionary::init;

            virtual status_t lookup(const LSPString *key, LSPString *value);

            virtual status_t lookup(const LSPString *key, IDictionary **value);

            virtual status_t get_value(size_t index, LSPString *key, LSPString *value);

            virtual status_t get_child(size_t index, LSPString *key, IDictionary **dict);

            virtual size_t size();

            virtual status_t init(const LSPString *path);

        public:
            /**
             * Clear dictionary contents
             */
            void        clear();

    };

} /* namespace lsp */

#endif /* CORE_I18N_ROOTDICTIONARY_H_ */
