# sanscript.js

[Sanscript](https://github.com/indic-transliteration/sanscript.js) is a transliteration library for Indian languages, supporting the most popular Indian scripts and several romanization schemes. It is authored by Arun Prasad and Sanskrit coders, and released under the MIT license.

## Usage in Scrite

Scrite uses sanscript.js for two purposes:

- As an additional transliteration option for users writing in Indian scripts.
- For cross-transliteration, enabling conversion of text between Indian scripts and romanization schemes within Scrite's transliteration workflows.

## Embedded Build

The file `sanscript-24e1510.js` was built from commit [24e1510](https://github.com/indic-transliteration/sanscript.js/commit/24e1510) of the upstream repository. It is compiled into the Scrite application binary as a Qt resource (accessible as `qrc:/sanscript.js/sanscript.js`) and evaluated at runtime using `QJSEngine`.

## License

Use of sanscript.js in Scrite is permitted by the [MIT License](https://github.com/indic-transliteration/sanscript.js/blob/master/LICENSE) under which it is released.
