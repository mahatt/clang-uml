# Troubleshooting

<!-- toc -->

* [General issues](#general-issues)
  * [`clang-uml` crashed when generating diagram](#clang-uml-crashed-when-generating-diagram)
  * [Diagram generation is very slow](#diagram-generation-is-very-slow)
  * [Diagram generated with PlantUML is cropped](#diagram-generated-with-plantuml-is-cropped)
  * [`clang` produces several warnings during diagram generation](#clang-produces-several-warnings-during-diagram-generation)
  * [Cannot generate diagrams from header-only projects](#cannot-generate-diagrams-from-header-only-projects)
  * [YAML anchors and aliases are not fully supported](#yaml-anchors-and-aliases-are-not-fully-supported)
* [Class diagrams](#class-diagrams)
  * ["fatal error: 'stddef.h' file not found"](#fatal-error-stddefh-file-not-found)
* [Sequence diagrams](#sequence-diagrams)
  * [Generated diagram is empty](#generated-diagram-is-empty)
  * [Generated diagram contains several empty control blocks or calls which should not be there](#generated-diagram-contains-several-empty-control-blocks-or-calls-which-should-not-be-there)

<!-- tocstop -->

## General issues

### `clang-uml` crashed when generating diagram

If `clang-uml` crashes with a segmentation fault, it is possible to trace the
exact stack trace of the fault using the following steps:

First, build `clang-uml` from source in debug mode, e.g.:
```bash
$ make debug
```

Then run `clang-uml`, preferably with `-vvv` for verbose log output. If your
`.clang-uml` configuration file contains more than 1 diagram, specify only
a single diagram to make it easier to trace the root cause of the crash, e.g.:

```bash
$ debug/src/clang-uml -vvv -n my_diagram
```

After `clang-uml` crashes again, detailed backtrace (generated using
[backward-cpp](https://github.com/bombela/backward-cpp)) should be visible
on the console.

If possible, [create an issue](https://github.com/bkryza/clang-uml/issues) and
paste the stack trace and few last logs from the console.

### Diagram generation is very slow

`clang-uml` uses Clang's [RecursiveASTVisitor](https://clang.llvm.org/doxygen/classclang_1_1RecursiveASTVisitor.html), to
traverse the source code. By default, this visitor is invoked on every
translation unit (i.e. each entry in your `compile_commands.json`), including
all of their header dependencies recursively. This means, that for large code
bases  with hundreds or thousands of translation units, traversing all of them
will be slow (think `clang-tidy` slow...).

Fortunately, in most practical cases it is not necessary to traverse the entire
source code for each diagram, as all the information necessary to generate
a single diagram usually can be found in just a few translation units, or even
a single one.

This is where the `glob` configuration parameter comes in. It can be used to
limit the number of translation units to visit for a given diagram, for instance:

```yaml
diagrams:
  ClassAContextDiagram:
    type: class
    ...
    glob:
      - src/classA.cpp
    ...
  InterfaceHierarchyDiagram:
    type: class
    ...
    glob:
      - src/interfaces/*.cpp
    ...
```

This should improve the generation times for individual diagrams significantly.

Furthermore, diagrams are generated in parallel if possible, by default using
as many threads as virtual CPU's are available on the system, however it can
be adjusted also manually using `-t` command line option.

### Diagram generated with PlantUML is cropped

When generating diagrams with PlantUML without specifying an output file format,
the default is PNG.
Unfortunately PlantUML will not check if the diagram will fit in the default PNG
size, and often the diagram
will be incomplete in the picture. A better option is to specify SVG as output
format and then convert
to PNG, e.g.:

```bash
$ plantuml -tsvg mydiagram.puml
$ convert +antialias mydiagram.svg mydiagram.png
```

### `clang` produces several warnings during diagram generation

During the generation of the diagram `clang` may report a lot of warnings, which
do not occur during the compilation with other compiler (e.g. GCC). This can be
fixed easily by using the `add_compile_flags` config option. For instance,
assuming that the warnings are as follows:

```
... warning: implicit conversion from 'int' to 'float' changes value from 2147483647 to 2147483648 [-Wimplicit-const-int-float-conversion]
... warning: declaration shadows a variable in namespace 'YAML' [-Wshadow]
```

simply add the following to your `.clang-uml` configuration file:

```
add_compile_flags:
  - -Wno-implicit-const-int-float-conversion
  - -Wno-shadow
```

Alternatively, the same can be passed through the `clang-uml` command line, e.g.

```bash
$ clang-uml --add-compile-flag -Wno-implicit-const-int-float-conversion \
            --add-compile-flag -Wno-shadow ...
```

Please note that if your `compile_commands.json` already contains - for instance
`-Wshadow` - then you also have to remove it, i.e.:

```
add_compile_flags:
  - -Wno-implicit-const-int-float-conversion
  - -Wno-shadow
remove_compile_flags:
  - -Wshadow
```

### Cannot generate diagrams from header-only projects

Currently, in order to generate UML diagrams using `clang-uml` it is necessary
that at least one translation unit (e.g. one `cpp`) exists and it is included
in the generated `compile_commands.json` database.

However, even if your project is a header only library, first check if the
generate `compile_commands.json` contains any entries - if yes you should be
fine - just make sure the `glob` pattern in the configuration file matches
any of them. This is due to the fact that most header only projects still have
test cases, which are compiled and executed, and which include the headers.
These are perfectly fine to be used as translation units to generate the
diagrams.

In case, the code really does not contain any translation units, you will have
to create one, typically a basic `main.cpp` which includes the relevant headers
should be fine.
Also, it's possible to simply create a separate project, with a single
translation unit, which includes the relevant headers and create diagrams
from there.

In the future there might be a workaround for this in `clang-uml`.

### YAML anchors and aliases are not fully supported

`clang-uml` uses [yaml-cpp](https://github.com/jbeder/yaml-cpp) library, which
currently does not support
[merging YAML anchor dictionaries](https://github.com/jbeder/yaml-cpp/issues/41),
e.g. in the following configuration file the `main_sequence_diagram` will work,
but the `foo_sequence_diagram` will fail with parse error:

```yaml
compilation_database_dir: debug
output_directory: output

.sequence_diagram_anchor: &sequence_diagram_anchor
  type: sequence
  glob: []
  start_from:
    - function: 'main(int,const char**)'

diagrams:
  main_sequence_diagram: *sequence_diagram_anchor
  foo_sequence_diagram:
    <<: *sequence_diagram_anchor
    glob: [src/foo.cc]
    start_from:
      - function: 'foo(int,float)'
```

One option around this is to use some YAML preprocessor, such as
[yq](https://github.com/mikefarah/yq) on such file and passing
the configuration file to `clang-uml` using `stdin`, e.g.:

```bash
yq 'explode(.)' .clang-uml | clang-uml --config -
```

## Class diagrams
### "fatal error: 'stddef.h' file not found"

This error means that Clang cannot find some standard headers in the include
paths  specified in the `compile_commands.json`. This typically happens on macOS
and sometimes on Linux, when the code was compiled with different Clang version
than `clang-uml` itself.

One solution to this issue is to add the following line to your `CMakeLists.txt`
file:

```cmake
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
```

Another option is to provide an option (on command line or in configuration
file) called `query_driver` (inspired by the [clangd](https://clangd.llvm.org/)
language  server - although much less flexible), which will invoke the
provider compiler command and query it for its default system paths, which then
will be added to each compile command in the database. This is especially useful
on macOS as well as for embedded toolchains, example usage:

```bash
$ clang-uml --query-driver arm-none-eabi-g++
```

Another option is to make sure that the Clang is installed on the system (even
if not used for building your
project), e.g.:

```bash
apt install clang
```

If this doesn't help to include paths can be customized using config options:

* `add_compile_flags` - which adds a list of compile flags such as include paths
  to each entry of the compilation database
* `remove_compile_flags` - which removes existing compile flags from each entry
  of the compilation database

For instance:

```yaml
add_compile_flags:
  - -I/opt/my_toolchain/include
remove_compile_flags:
  - -I/usr/include
```

These options can be also passed on the command line, for instance:

```bash
$ clang-uml --add-compile-flag -I/opt/my_toolchain/include \
            --remove-compile-flag -I/usr/include ...
```

## Sequence diagrams
### Generated diagram is empty

In order to generate sequence diagram the `start_from` configuration option must
have a valid starting point
for the diagram (e.g. `function`), which must match exactly the function
signature in the `clang-uml` model.
Look for error in the console output such as:

```bash
Failed to find participant mynamespace::foo(int) for start_from condition
```

which means that either you have a typo in the function signature in the
configuration file, or that the function
was not defined in the translation units you specified in the `glob` patterns
for this diagram. Run again the
`clang-uml` tool with `-vvv` option and look in the console output for any
mentions of the function from
which the diagram should start and copy the exact signature into the
configuration file.

### Generated diagram contains several empty control blocks or calls which should not be there

Currently the filtering of call expressions and purging empty control blocks (
e.g. loops or conditional statements),
within which no interesting calls were included in the diagram is not perfect.
In case the regular `namespaces` filter
is not enough, it is useful to add also a `paths` filter, which will only
include participants and call expressions
from files in a subdirectory of your project, e.g.:

```yaml
   include:
     namespaces:
       - myproject
     paths:
       - src
```
