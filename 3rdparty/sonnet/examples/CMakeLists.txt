add_executable(dialog dialogexample.cpp)
target_link_libraries(dialog KF6::SonnetCore KF6::SonnetUi)

add_executable(highlighter highlighterexample.cpp)
target_link_libraries(highlighter KF6::SonnetCore KF6::SonnetUi)

add_executable(configdialog configdialog.cpp)
target_link_libraries(configdialog KF6::SonnetCore KF6::SonnetUi)

add_executable(dictionarycombobox dictionarycombobox.cpp)
target_link_libraries(dictionarycombobox KF6::SonnetCore KF6::SonnetUi)

add_executable(textedit textedit.cpp)
target_link_libraries(textedit KF6::SonnetCore KF6::SonnetUi)

add_executable(plaintextedit plaintextedit.cpp)
target_link_libraries(plaintextedit KF6::SonnetCore KF6::SonnetUi)

