========
Overview
========

.. 2015/11/13: initial version

What is Zeus
============

Zeus is primarily a configuration compiler. The goals is to provide performant, scalable, and
developer-friendly APIs for configuration data. There are also tools to turn Zeus into a 
REST API.

Terminology
===========

Here is a common set of terms that are used throughout this document:

- **JSON** - a commonly used data interchange format. Here is the `JSON spec <http://json.org/>`_ and there is also an `online JSON editor <http://jsoneditoronline.org/>`_
- **YAML** - a superset of JSON which is easier for humans to generate. Here is the latest `YAML 1.2 spec <http://www.yaml.org/spec/1.2/spec.html>`_ and there is also an `online YAML editor <http://yaml-online-parser.appspot.com/>`_
- **Source Config** - raw YAML-based configuration for example `these files <https://github.com/yahoo/zeus/tree/master/conf>`_
- **Compiled Config** - the output of the component job, typically in the form of native code
- **Config Version** - a build-time version of the compiled configuration, currently a UNIX epoch timestamp
- **Config Context** - also known as the config dimensions, a set of key/value pairs which describes the request
- **Config Keys** - statically typed variables which vary on the config context
- **Config Regular Expressions** - regexes can be defined to match configuration values
- **Config Sets** - lists (or sets) can be defined to match configuration values
