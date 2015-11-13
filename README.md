# Zeus Configuration Compiler

[![Build Status](https://travis-ci.org/yahoo/zeus.svg)](https://travis-ci.org/yahoo/zeus)

Compiles YAML input file into source code.

Currently, the following languages are supported:
 * C++
 * Json (through C++)
 * Java
 * JavaScript
 * PHP

## Build Dependencies
 * C++ compiler with C++11 support
   * gcc >= 4.8
   * clang >= 3.3
 * boost >= 1.49
 * cmake
 * [https://github.com/jbeder/yaml-cpp](yamlcpp) (already a submodule)

## Example
```
---
- dimensions:
  - lang

---
- settings: master
  date_format: YYYY.MM.DD

---
- settings: { lang: en-US }
  date_format: MM-DD-YYYY
- settings: { lang: pt-BR }
  date_format: DD/MM/YYYY
```

generates the following _JavaScript_ code:

```
function date_format() {
	switch (this.lang) {
	case en_US:
		return “MM-DD-YYYY”;
	case pt_BR:
		return “DD/MM/YYYY”;
	default:
		return “YYYY.MM.DD”;
	}
};
```
## Compiler's architectural diagram
![Diagram](doc/compiler-arch.png?raw=true)
