# `@SR{ Markup }`

This is a simple markup language (in the same vein as markdown) for structured
document writing.

## Overview

The project is separated into a frontend and a backend. 

The frontend is an executable written in Rust, it parses the document creating an 
AST (Abstract Syntax Tree) of the document to be passed to the dynamically loaded backend.

The backend is a shared library that is loaded by the frontend at runtime that 
uses the AST to write a desired output format.
These functions are expected to be exported symbols in the shared library:
(**All function are optional**)

  - `void* srMarkupInit(const Arguments* args)`
    - Called once at startup, you can return a pointer to some user data that will be passed into the other functions.
    - The `args` parameter contains all of the command line arguments passed into the `sr-markup` program.
      - It is a valid pointer throughout the lifetime of the program.
  - `void  srMarkupProcess(const ASTNode* node, void* user_data)`
    - Called for each top level AST node, the user_data is what you returned from the `srMarkupInit` function.
  - `void  srMarkupShutdown(void* user_data)`
    - Called once on shutdown of the app for doing any cleanup you want to do, the user_data is what you returned from the `srMarkupInit` function.

If you are using C/++ for the backend there is a provided `sr-markup.h` file for easy reading of the AST.

Alternatively a backend can be implemented in Rust by modifying `main.rs` and 
implementing the `IASTProcessor` trait.

## Basic Usage

The 'Release' folder contains the compiled Windows x64 executable and 
the C header with the API.

The `example-backend` folder has a hastily written example of reading the AST
to generate a json file with the same information.

```bash
# By default the output of the program will go to standard out.
# Errors will be printed out into standard error.

# This will print out th AST with the default debug printer.
sr-markup -i InputDocument.srmark

# This will use the functions in `Library.dll` to do the processing of the AST.
sr-markup -i InputDocument.srmark -l Library.dll
```

If you want some more information on the various flags just do `sr-markup -h`.

## Syntax Example

```swift
@Header(
  Title      = "Blog Post Title", 
  CoverImage = "url(data/post-data/07/cover.png)",
  Date       = "December 5th, 2019",
)

@h3 { - Header Text - }

@video(Size = "Full", Src = "data/post-data/07/trailer.mp4")

@Text(Size = "Full") {
  This is some dummy text!
}

@p (FontSize = "20px") 
{
  This is a tag with a parameter.

  @MyNestedTag {
    Helloooo
  }
}

This is some top level text.
```

**IMPORTANT: These characters must be escaped (e.g \\@) within text blocks: '@', '{', '}', and '='.**

_Whitespace is not significant._

## Terminology and Syntax

### TagNode
These are the main building block of the format. They contain other `Node` types
and have a list of [TagAttribute](#TagAttribute)s.
The list of `TagAttribute`s on `TagNode`s can havea trailing comma.
```swift
@TagWithNoContentOrAttributes

@TagWithNoAttributes { Some Content }

@TagWithNoContent(Attrib = "Hello")
```


### TagAttribute
This is a pairing of a string name and a [LiteralNode](#LiteralNode).
```swift
@ExampleTag(
  Attribute1Name = "A String value",
  Attribute2Name = false,
  Attribute3Name = 42.32,
)
```

### TextNode
Simple block of text.
```
Text is a just set of characters, make sure to escape the special
characters :\).
```
### LiteralNode
Variant consisted of either a string (`StringView`), number (`f64`), or a boolean (`bool`) value.
```swift
String  = "Strings are In Double Quotes"
Number  = 1.5
Boolean = true / false
```

## License

```
MIT License

Copyright (c) 2021 Shareef Abdoul-Raheem

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
