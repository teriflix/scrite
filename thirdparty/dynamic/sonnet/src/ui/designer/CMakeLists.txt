include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(Sonnet::DictionaryComboBox
    TOOLTIP "Dictionary Combobox"
    WHATSTHIS "A combobox to select a dictionary for spellchecking"
    GROUP "Sonnet (KF6)"
)

ecm_add_qtdesignerplugin(sonnetuiwidgets
    NAME SonnetUiWidgets
    OUTPUT_NAME sonnet6widgets
    WIDGETS
        Sonnet::DictionaryComboBox
    LINK_LIBRARIES
        KF6::SonnetUi
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
