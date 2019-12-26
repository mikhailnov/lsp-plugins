/*
 * CtlLabel.h
 *
 *  Created on: 7 июл. 2017 г.
 *      Author: sadko
 */

#ifndef UI_CTL_CTLLABEL_H_
#define UI_CTL_CTLLABEL_H_

namespace lsp
{
    namespace ctl
    {
        enum ctl_label_type_t
        {
            CTL_LABEL_TEXT,
            CTL_LABEL_VALUE,
            CTL_LABEL_PARAM,
            CTL_STATUS_CODE
        };


        class CtlLabel: public CtlWidget
        {
            public:
                static const ctl_class_t metadata;

            protected:
                class PopupWindow: public LSPWindow
                {
                    private:
                        friend class CtlLabel;

                    protected:
                        CtlLabel   *pLabel;
                        LSPBox      sBox;
                        LSPEdit     sValue;
                        LSPLabel    sUnits;
                        LSPButton   sApply;
                        LSPButton   sCancel;

                    public:
                        explicit PopupWindow(CtlLabel *label, LSPDisplay *dpy);
                        virtual ~PopupWindow();

                        virtual status_t init();
                        virtual void destroy();
                };

            protected:
                CtlColor            sColor;
                CtlPort            *pPort;
                ctl_label_type_t    enType;
                float               fValue;
                bool                bDetailed;
                bool                bSameLine;
                ssize_t             nUnits;
                ssize_t             nPrecision;
                PopupWindow        *pPopup;

            protected:
                static status_t slot_submit_value(LSPWidget *sender, void *ptr, void *data);
                static status_t slot_change_value(LSPWidget *sender, void *ptr, void *data);
                static status_t slot_cancel_value(LSPWidget *sender, void *ptr, void *data);
                static status_t slot_dbl_click(LSPWidget *sender, void *ptr, void *data);
                static status_t slot_key_up(LSPWidget *sender, void *ptr, void *data);
                static status_t slot_mouse_button(LSPWidget *sender, void *ptr, void *data);

            protected:
                void            commit_value();
                bool            apply_value(const LSPString *value);

            public:
                explicit CtlLabel(CtlRegistry *src, LSPLabel *widget, ctl_label_type_t type);
                virtual ~CtlLabel();

            public:
                /** Begin initialization of controller
                 *
                 */
                virtual void init();

                /** Set attribute to widget
                 *
                 * @param att attribute identifier
                 * @param value attribute value
                 */
                virtual void set(widget_attribute_t att, const char *value);

                /** Notify controller about one of port bindings has changed
                 *
                 * @param port port triggered change
                 */
                virtual void notify(CtlPort *port);

                /** Complete initialization
                 *
                 */
                virtual void end();
        };
    
    } /* namespace ctl */
} /* namespace lsp */

#endif /* UI_CTL_CTLLABEL_H_ */
