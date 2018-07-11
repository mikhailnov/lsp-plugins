/*
 * Gtk2Window.cpp
 *
 *  Created on: 20 окт. 2015 г.
 *      Author: sadko
 */

#include <ui/gtk2/ui.h>

namespace lsp
{
    Gtk2Container::Gtk2Container(plugin_ui *ui, widget_t w_class):
        Gtk2Widget(ui, w_class)
    {
        nAdded      = 0;
        sBgColor.set(pUI->theme(), C_BACKGROUND);
    }

    Gtk2Container::~Gtk2Container()
    {
    }

    void Gtk2Container::set(widget_attribute_t att, const char *value)
    {
        switch (att)
        {
            case A_BG_COLOR:
                sBgColor.set(pUI->theme(), value);
                break;
            default:
                Gtk2Widget::set(att, value);
                break;
        }
    }

    void Gtk2Container::end()
    {
        if (pWidget != NULL)
        {
            GdkColor col;
            col.red         = sBgColor.red() * 0xffff;
            col.green       = sBgColor.green() * 0xffff;
            col.blue        = sBgColor.blue() * 0xffff;
            col.pixel       = 0;
            gtk_widget_modify_bg(pWidget, GTK_STATE_NORMAL, &col);
        }

        Gtk2Widget::end();
    }

} /* namespace lsp */