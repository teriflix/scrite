set_source_files_properties("misc/../js/getBoxToBoxArrow.js"
  PROPERTIES QT_RESOURCE_ALIAS "getBoxToBoxArrow.js"
)

qt_add_resources(Scrite SCRITE_MISC_RESOURCES_1
  PREFIX "/dragonman225-curved-arrows"
  BASE "misc"
  FILES
    "misc/../js/getBoxToBoxArrow.js"
)

qt_add_resources(Scrite SCRITE_MISC_RESOURCES_2
  PREFIX "/misc"
  BASE "misc"
  FILES
    "misc/annotations_metadata.json"
    "misc/fetchogattribs.js"
    "misc/structure_groups.lst"
    "misc/scrited_closing_frame_video.mp4"
    "misc/scriptalay_info.md"
    "misc/homescreen_info.md"
)

set_source_files_properties("misc/../../../LICENSE.txt"
  PROPERTIES QT_RESOURCE_ALIAS "LICENSE.txt"
)

set_source_files_properties("misc/../../../thirdparty/source/quill/quill.min.js"
  PROPERTIES QT_RESOURCE_ALIAS "quill/quill.min.js"
)

set_source_files_properties("misc/../../../thirdparty/source/quill/quill.snow.css"
  PROPERTIES QT_RESOURCE_ALIAS "quill/quill.snow.css"
)

qt_add_resources(Scrite SCRITE_MISC_RESOURCES_4
  PREFIX "/"
  BASE "misc"
  FILES
    "misc/richtexteditor.html"
    "misc/qwebchannel.js"
    "misc/richtexttransform.html"
)

qt_add_resources(Scrite SCRITE_MISC_RESOURCES_3
  PREFIX "/"
  FILES
    "misc/../../../LICENSE.txt"
    "misc/../../../thirdparty/source/quill/quill.min.js" 
    "misc/../../../thirdparty/source/quill/quill.snow.css"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_BENGALI
  PREFIX "/fonts/Bengali"
  BASE "fonts/Bengali"
  FILES
    "fonts/Bengali/HindSiliguri-Bold.ttf"
    "fonts/Bengali/HindSiliguri-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_ENGLISH
  PREFIX "/fonts/English"
  BASE "fonts/English"
  FILES
    "fonts/English/CourierPrime-Bold.ttf"
    "fonts/English/CourierPrime-BoldItalic.ttf"
    "fonts/English/CourierPrime-Italic.ttf"
    "fonts/English/CourierPrime-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_GUJARATI
  PREFIX "/fonts/Gujarati"
  BASE "fonts/Gujarati"
  FILES
    "fonts/Gujarati/HindVadodara-Bold.ttf"
    "fonts/Gujarati/HindVadodara-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_HINDI
  PREFIX "/fonts/Hindi"
  BASE "fonts/Hindi"
  FILES
    "fonts/Hindi/Mukta-Bold.ttf"
    "fonts/Hindi/Mukta-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_KANNADA
  PREFIX "/fonts/Kannada"
  BASE "fonts/Kannada"
  FILES
    "fonts/Kannada/BalooTamma2-Bold.ttf"
    "fonts/Kannada/BalooTamma2-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_MALAYALAM
  PREFIX "/fonts/Malayalam"
  BASE "fonts/Malayalam"
  FILES
    "fonts/Malayalam/BalooChettan2-Bold.ttf"
    "fonts/Malayalam/BalooChettan2-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_MARATHI
  PREFIX "/fonts/Marathi"
  BASE "fonts/Marathi"
  FILES
    "fonts/Marathi/Shusha-Normal.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_ORIYA
  PREFIX "/fonts/Oriya"
  BASE "fonts/Oriya"
  FILES
    "fonts/Oriya/BalooBhaina2-Bold.ttf"
    "fonts/Oriya/BalooBhaina2-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_PUNJABI
  PREFIX "/fonts/Punjabi"
  BASE "fonts/Punjabi"
  FILES
    "fonts/Punjabi/BalooPaaji2-Bold.ttf"
    "fonts/Punjabi/BalooPaaji2-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_RUBIK
  PREFIX "/fonts/Rubik"
  BASE "fonts/Rubik"
  FILES
    "fonts/Rubik/Rubik-BoldItalic.ttf"
    "fonts/Rubik/Rubik-Regular.ttf"
    "fonts/Rubik/Rubik-Italic.ttf"
    "fonts/Rubik/Rubik-Bold.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_SANSKRIT
  PREFIX "/fonts/Sanskrit"
  BASE "fonts/Sanskrit"
  FILES
    "fonts/Sanskrit/Mukta-Bold.ttf"
    "fonts/Sanskrit/Mukta-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_TAMIL
  PREFIX "/fonts/Tamil"
  BASE "fonts/Tamil"
  FILES
    "fonts/Tamil/HindMadurai-Bold.ttf"
    "fonts/Tamil/HindMadurai-Regular.ttf"
)

qt_add_resources(Scrite SCRITE_FONT_RESOURCES_TELUGU
  PREFIX "/fonts/Telugu"
  BASE "fonts/Telugu"
  FILES
    "fonts/Telugu/HindGuntur-Bold.ttf"
    "fonts/Telugu/HindGuntur-Regular.ttf"
)

