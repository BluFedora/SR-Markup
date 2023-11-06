# `@SR{ Markup }`

This is a simple markup language (in the same vein as markdown) for structured document writing.

## Overview

This is a library for a parsing custom markup language.

`src/bin/ast_dumper.rs` contains a simple example of parsing the srmark file
along with processing it with a very basic debug dumper implementation of a `IASTProcessor`.

```bash
# By default the output of the program will go to standard out.
# Errors will be printed out into standard error.

# This will print out the AST with the default debug printer.
sr-markup-ast-dump --input InputDocument.srmark
```

If you want some more information on the various flags just do `sr-markup-ast-dump -h`.

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

  @"A Fun Tag With Spaces In The Name"()
}

@p (FontSize = "20px") 
{
  This is a tag with an attribute name "FontSize".

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

@"Tag With Spaces In The Name"
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
If an attribute that already exists is listed later in the list then 
the node will contain the value of the latest listing.


### TextNode
Simple block of text.
```
Text is a just set of characters, make sure to escape the special
characters :\).
```
### LiteralNode
Variant consisted of either a string (`String`), number (`f64`), or a boolean (`bool`) value.
```swift
String  = "Strings are In Double Quotes"
Number  = 1.5
Boolean = true / false
```
**IMPORTANT only integer values in the range [-2^53, 2^53] can be properly represented.**

## License

```
MIT License

Copyright (c) 2020-2023 Shareef Abdoul-Raheem

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
