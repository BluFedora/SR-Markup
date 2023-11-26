use srmarkup::{self, ASTNodeText};

use structopt::StructOpt;

use minijinja::{context, Environment};

use std::fs::File;
use std::io::Read;

pub mod html {
    use std::collections::HashMap;
    use std::io::Write;

    pub type ElementTagID = u32;

    pub struct ElementTag {
        pub start_tag: String,
        pub end_tag: Option<String>, // Same as `start_tag` if not specified.
        pub is_void_element: bool,
    }

    pub type ElementID = u32;

    pub struct Document {
        pub elements: Vec<Element>,
        pub tags: Vec<ElementTag>,
        pub doc_type: ElementID,
        pub html: ElementID,
        pub head: ElementID,
        pub body: ElementID,
    }

    pub enum ElementContent {
        Element(ElementID),
        Text(String),
    }

    pub struct Element {
        pub tag: ElementTagID,
        pub attributes: HashMap<String, String>,
        pub contents: Vec<ElementContent>,
        pub is_comment: bool, // Print will be wrapped in "<!--" and "-->".
    }

    impl Element {
        pub fn render(&self, writer: &mut dyn Write, doc: &Document) {
            let tag_data = &doc.tags[self.tag as usize];

            if self.is_comment {
                let _ = write!(writer, "<!--\n");
            }

            let _ = write!(writer, "<{}", tag_data.start_tag);

            for attrib in self.attributes.iter() {
                let _ = write!(writer, " {}", attrib.0);

                if !attrib.1.is_empty() {
                    let _ = write!(writer, "=\"{}\"", attrib.1);
                }
            }

            let _ = write!(writer, ">\n");

            if !tag_data.is_void_element {
                for item in self.contents.iter() {
                    match item {
                        ElementContent::Element(element_id) => {
                            doc.elements[*element_id as usize].render(writer, doc);
                        }
                        ElementContent::Text(txt) => {
                            let _ = write!(writer, "{}", txt);
                        }
                    }
                }

                if tag_data.end_tag.is_some() {
                    let _ = write!(writer, "\n</{}>\n", tag_data.end_tag.as_ref().unwrap());
                } else {
                    let _ = write!(writer, "\n</{}>\n", tag_data.start_tag);
                }
            }

            if self.is_comment {
                let _ = write!(writer, "-->\n");
            }
        }
    }

    impl Document {
        pub fn create_element<S: AsRef<str>>(&mut self, tag: S) -> ElementID {
            let tag_str_ref = tag.as_ref();

            let element = Element {
                tag: self.tag_id_from_string(tag_str_ref),
                attributes: Default::default(),
                contents: Default::default(),
                is_comment: false,
            };
            let id = self.elements.len() as ElementID;
            self.elements.push(element);
            return id;
        }

        pub fn get_element_by_id(&mut self, element_id: ElementID) -> &mut Element {
            return &mut self.elements[element_id as usize];
        }

        pub fn get_const_element_by_id(&self, element_id: ElementID) -> &Element {
            return &self.elements[element_id as usize];
        }

        pub fn set_attribute(&mut self, element_id: ElementID, key: &String, value: String) -> () {
            let _ = self
                .get_element_by_id(element_id)
                .attributes
                .insert(key.to_lowercase(), value);
        }

        pub fn remove_attribute(&mut self, element_id: ElementID, key: &String) {
            self.get_element_by_id(element_id).attributes.remove(key);
        }

        pub fn insert_content_at(
            &mut self,
            element_id: ElementID,
            index: usize,
            element: ElementContent,
        ) -> () {
            self.get_element_by_id(element_id)
                .contents
                .insert(index, element);
        }

        pub fn push_content(&mut self, element_id: ElementID, element: ElementContent) -> () {
            self.insert_content_at(
                element_id,
                self.elements[element_id as usize].contents.len(),
                element,
            );
        }

        pub fn remove_content(&mut self, element_id: ElementID, index: usize) {
            self.get_element_by_id(element_id).contents.remove(index);
        }

        pub fn set_is_comment(&mut self, element_id: ElementID, value: bool) -> () {
            self.get_element_by_id(element_id).is_comment = value;
        }

        pub fn tag_id_from_string(&mut self, tag_str: &str) -> ElementTagID {
            let lower_case_tag = String::from(tag_str).to_lowercase();
            let it = self
                .tags
                .binary_search_by(|x| x.start_tag.cmp(&lower_case_tag));

            match it {
                Ok(id_index) => return id_index as ElementTagID,
                Err(insertion_index) => {
                    let id = self.tags.len() as ElementID;
                    self.tags.insert(
                        insertion_index,
                        ElementTag {
                            start_tag: lower_case_tag,
                            end_tag: Option::None,
                            is_void_element: false,
                        },
                    );
                    return id;
                }
            }
        }

        pub fn render(&self, writer: &mut dyn Write, element_id: ElementID) {
            self.elements[element_id as usize].render(writer, self);
        }

        pub fn render_content(&self, writer: &mut dyn Write, element_content: &ElementContent) {
            match element_content {
                ElementContent::Element(element_id) => {
                    self.render(writer, *element_id);
                }
                ElementContent::Text(txt) => {
                    let _ = write!(writer, "{}", txt);
                }
            }
        }
    }

    impl Default for Document {
        fn default() -> Self {
            let mut result = Self {
                elements: Default::default(),

                tags: vec![
                    // Resources:
                    //   - [Element Tag Listing](https://developer.mozilla.org/en-US/docs/Web/HTML/Element)
                    //   - [Void Elements](https://developer.mozilla.org/en-US/docs/Glossary/Void_element)

                    // Document Type
                    ElementTag {
                        start_tag: String::from("!doctype"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    // Root Element
                    ElementTag {
                        start_tag: String::from("html"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Document Metadata
                    ElementTag {
                        start_tag: String::from("base"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("head"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("link"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("meta"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("style"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("title"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Sectioning Root
                    ElementTag {
                        start_tag: String::from("body"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Content Sectioning
                    ElementTag {
                        start_tag: String::from("address"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("article"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("aside"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("footer"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("header"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("h1"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("h1"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("h2"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("h3"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("h4"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("h5"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("h6"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("nav"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("hgroup"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("section"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("search"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Text Content
                    ElementTag {
                        start_tag: String::from("blockquote"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("dd"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("div"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("dt"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("figcaption"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("figure"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("hr"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("li"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("menu"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("ol"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("p"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("pre"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("ul"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Inline Text Semantics
                    ElementTag {
                        start_tag: String::from("a"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("abbr"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("b"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("bdi"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("bdo"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("br"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("cite"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("code"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("data"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("dfn"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("em"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("i"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("kbd"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("mark"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("q"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("rp"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("rt"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("ruby"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("s"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("samp"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("small"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("span"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("strong"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("sub"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("sup"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("time"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("u"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("var"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("wbr"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    // Image and Multimedia
                    ElementTag {
                        start_tag: String::from("area"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("audio"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("img"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("map"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("track"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("video"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Embedded Content
                    ElementTag {
                        start_tag: String::from("embed"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("iframe"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("object"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("picture"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("portal"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("source"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    // SVG and MathML
                    ElementTag {
                        start_tag: String::from("svg"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("math"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Scripting
                    ElementTag {
                        start_tag: String::from("canvas"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("noscript"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("script"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Demarcating Edits
                    ElementTag {
                        start_tag: String::from("del"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("ins"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Table Content
                    ElementTag {
                        start_tag: String::from("caption"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("col"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("colgroup"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("table"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("tbody"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("td"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("tfoot"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("th"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("thead"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("tr"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Forms
                    ElementTag {
                        start_tag: String::from("button"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("datalist"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("fieldset"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("form"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("input"),
                        end_tag: Option::None,
                        is_void_element: true,
                    },
                    ElementTag {
                        start_tag: String::from("label"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("legend"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("meter"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("optgroup"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("option"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("output"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("progress"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("select"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("textarea"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Interactive Elements
                    ElementTag {
                        start_tag: String::from("details"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("dialog"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("summary"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    // Web Components
                    ElementTag {
                        start_tag: String::from("slot"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                    ElementTag {
                        start_tag: String::from("template"),
                        end_tag: Option::None,
                        is_void_element: false,
                    },
                ],
                doc_type: Default::default(),
                html: Default::default(),
                head: Default::default(),
                body: Default::default(),
            };

            result
                .tags
                .sort_unstable_by(|a: &ElementTag, b: &ElementTag| {
                    a.start_tag.partial_cmp(&b.start_tag).unwrap()
                });

            result.doc_type = result.create_element("!doctype");
            result.html = result.create_element("html");
            result.head = result.create_element("head");
            result.body = result.create_element("body");

            result.set_attribute(result.doc_type, &"html".to_string(), "".to_string());
            result.insert_content_at(result.doc_type, 0, ElementContent::Element(result.html));

            result.set_attribute(result.html, &"lang".to_string(), "en".to_string());
            result.insert_content_at(result.html, 0, ElementContent::Element(result.head));
            result.insert_content_at(result.html, 1, ElementContent::Element(result.body));

            let meta_charset = result.create_element("meta");
            let meta_viewport = create_meta_tag(
                &mut result,
                "viewport".to_string(),
                "width=device-width, initial-scale=1.0".to_string(),
            );
            let meta_xua = result.create_element("meta");

            result.set_attribute(meta_charset, &"charset".to_string(), "utf-8".to_string());
            result.set_attribute(meta_viewport, &"name".to_string(), "viewport".to_string());
            result.set_attribute(
                meta_viewport,
                &"content".to_string(),
                "width=device-width, initial-scale=1.0".to_string(),
            );
            result.set_attribute(
                meta_xua,
                &"http-equiv".to_string(),
                "X-UA-Compatible".to_string(),
            );
            result.set_attribute(meta_xua, &"content".to_string(), "ie=edge".to_string());

            result.push_content(result.head, ElementContent::Element(meta_charset));
            result.push_content(result.head, ElementContent::Element(meta_viewport));
            result.push_content(result.head, ElementContent::Element(meta_xua));

            return result;
        }
    }

    pub fn create_meta_tag(doc: &mut Document, name: String, content: String) -> ElementID {
        let result = doc.create_element("meta");

        doc.set_attribute(result, &"name".to_string(), name);
        doc.set_attribute(result, &"content".to_string(), content);

        return result;
    }

    pub fn escape(str: &str) -> String {
        return str
            .replace("&", "&amp;")
            .replace("<", "&lt;")
            .replace(">", "&gt;")
            .replace("\"", "&quot;")
            .replace("'", "&#039;");
    }
}

#[derive(Debug, StructOpt)]
struct Options {
    #[structopt(long, default_value = "TestInput.srmark")]
    pub input: String,
}

fn load_entire_file(file_name: &str) -> String {
    let mut result = String::new();
    let file = File::open(file_name);

    match file {
        Ok(mut file) => {
            let source_size = file.read_to_string(&mut result);

            match source_size {
                Ok(_) => {}
                Err(msg) => {
                    eprintln!("[ERROR] Failed to read file ('{}'), {}.", file_name, msg);
                }
            }
        }
        Err(msg) => {
            eprintln!("[ERROR] Failed to load file ('{}'), {}.", file_name, msg);
        }
    };

    return result;
}

fn main() {
    let options = Options::from_args();
    let input_path = &options.input;
    let input_source = load_entire_file(input_path);
    let lexer = srmarkup::Lexer::new(input_source);
    let mut parser = srmarkup::Parser::new(lexer);
    let parse_result: srmarkup::ParseResult = parser.parse();

    match parse_result {
        Ok(root_node) => {
            // title
            // theme
            // background_image
            // date
            // post_content
            let blog_post_template = load_entire_file("blog_post_template.html");
            let mut processor: HTMLProcessor = HTMLProcessor::new();
            srmarkup::visit_ast(&root_node, &mut processor);

            let doc = &processor.doc;
            let mut post_content = std::io::BufWriter::new(Vec::new());

            for body_content in doc.get_const_element_by_id(doc.body).contents.iter() {
                doc.render_content(&mut post_content, body_content);
            }

            let bytes = post_content.into_inner().unwrap();
            let post_content = String::from_utf8(bytes).unwrap();

            let ctx_vars = context! {
              title => processor.title,
              theme => processor.theme,
              background_image => processor.cover_image,
              date => processor.date,
              post_content => post_content,
            };

            let mut env = Environment::new();
            env.add_template("blog_post_template.html", blog_post_template.as_str())
                .unwrap();

            let main_template = env.get_template("blog_post_template.html").unwrap();

            println!("{}", main_template.render(ctx_vars).unwrap());
        }
        Err(error_log) => {
            eprintln!("Parse Error:");
            for err in &error_log.errors {
                eprintln!("  Line({}): {}\n", err.line_number, err.message);
            }
        }
    }
}

struct HTMLProcessor {
    doc: html::Document,
    title: String,
    cover_image: String,
    date: String,
    theme: String,
    element_stack: Vec<html::ElementID>,
}

impl HTMLProcessor {
    pub fn new() -> Self {
        HTMLProcessor {
            doc: Default::default(),
            title: Default::default(),
            cover_image: Default::default(),
            date: Default::default(),
            theme: Default::default(),
            element_stack: vec![],
        }
    }

    fn push_element(self: &mut Self, element: html::ElementID) {
        self.element_stack.push(element);
    }

    fn pop_element(self: &mut Self) {
        self.element_stack.pop();
    }

    fn remap_tag(tag: &str) -> &str {
        return match tag {
            "text" => "p",
            "image" => "img",
            "link" => "a",
            "ulist" => "ul",
            "olist" => "ol",
            "listitem" => "li",
            _ => tag,
        };
    }

    fn find_attribute_str(tag_node: &srmarkup::ASTNodeTag, key: &str) -> String {
        let attrib = tag_node.find_attribute(key);

        if attrib.is_some() {
            return attrib.unwrap().to_string();
        }

        return "".to_string();
    }

    fn extract_classes(tag_node: &srmarkup::ASTNodeTag) -> String {
        let css_class = tag_node.find_attribute("Class");
        let css_size = tag_node.find_attribute("Size");

        let mut result: String = Default::default();

        if css_class.is_some() {
            result.push_str(&css_class.unwrap().to_string());
        }

        if css_size.is_some() {
            let css_size = css_size.unwrap().to_string().to_lowercase();

            let append = match css_size.as_str() {
                "full" => " post-full",
                "half" => " post-half",
                _ => "",
            };

            result.push_str(append);
        }

        result.push_str("");

        result
    }
}

impl srmarkup::IASTProcessor for HTMLProcessor {
    fn visit_begin_root(&mut self, _: &srmarkup::ASTNodeRoot) -> srmarkup::ASTProcessorVisitResult {
        self.push_element(self.doc.body);
        return srmarkup::ASTProcessorVisitResult::Continue;
    }

    fn visit_begin_tag(
        &mut self,
        tag_node: &srmarkup::ASTNodeTag,
    ) -> srmarkup::ASTProcessorVisitResult {
        let tag_text = tag_node.text.to_lowercase();

        match tag_text.as_str() {
            "header" => {
                self.title = HTMLProcessor::find_attribute_str(tag_node, "Title");
                self.cover_image = HTMLProcessor::find_attribute_str(tag_node, "CoverImage");
                self.date = HTMLProcessor::find_attribute_str(tag_node, "Date");
                self.theme = HTMLProcessor::find_attribute_str(tag_node, "Theme");

                return srmarkup::ASTProcessorVisitResult::SkipChildren;
            }
            "file" => {
                let file_path = tag_node.find_attribute("Source");

                if file_path.is_some() {
                    let file_path_string = file_path.unwrap().to_string();
                    let file = File::open(&file_path_string);

                    match file {
                        Ok(mut file) => {
                            let mut source = String::new();
                            let source_size = file.read_to_string(&mut source);

                            match source_size {
                                Ok(_) => {
                                    self.visit_text(&ASTNodeText { text: source });
                                }
                                Err(msg) => {
                                    eprintln!(
                                        "[ERROR] Failed to read file ('{}'), {}.",
                                        file_path_string, msg
                                    );
                                }
                            }
                        }
                        Err(msg) => {
                            eprintln!(
                                "[ERROR] Failed to load file ('{}'), {}.",
                                file_path_string, msg
                            );
                        }
                    }
                }

                return srmarkup::ASTProcessorVisitResult::SkipChildren;
            }
            raw_tag => {
                let real_tag = HTMLProcessor::remap_tag(raw_tag);
                let css_classes = HTMLProcessor::extract_classes(&tag_node);
                let css_id = tag_node.find_attribute("ID");
                let src = tag_node.find_attribute("Src");

                let element = self.doc.create_element(real_tag);

                let is_video = real_tag == "video";

                if !css_classes.is_empty() {
                    self.doc
                        .set_attribute(element, &"class".to_string(), css_classes);
                }

                if css_id.is_some() {
                    self.doc
                        .set_attribute(element, &"id".to_string(), css_id.unwrap().to_string());
                }

                if is_video {
                    self.doc
                        .set_attribute(element, &"controls".to_string(), "".to_string());
                }

                if src.is_some() {
                    let source_string = src.unwrap().to_string();

                    if is_video {
                        let source_element = self.doc.create_element("source");
                        self.doc.set_attribute(
                            source_element,
                            &"type".to_string(),
                            "video/mp4".to_string(),
                        );
                        self.doc
                            .set_attribute(source_element, &"src".to_string(), source_string);

                        self.doc
                            .push_content(element, html::ElementContent::Element(source_element));
                    } else if real_tag == "img" {
                        self.doc
                            .set_attribute(element, &"src".to_string(), source_string);
                        self.doc.set_attribute(
                            element,
                            &"alt".to_string(),
                            HTMLProcessor::find_attribute_str(tag_node, "Alt"),
                        );
                    } else if real_tag == "a" {
                        self.doc
                            .set_attribute(element, &"href".to_string(), source_string);
                    }
                }

                let current_element = *self.element_stack.last().unwrap();

                self.doc
                    .push_content(current_element, html::ElementContent::Element(element));

                self.push_element(element);
            }
        }

        return srmarkup::ASTProcessorVisitResult::Continue;
    }

    fn visit_text(
        &mut self,
        text_node: &srmarkup::ASTNodeText,
    ) -> srmarkup::ASTProcessorVisitResult {
        let current_element = *self.element_stack.last().unwrap();
        let sanitized_string = html::escape(text_node.text.as_str());

        self.doc.push_content(
            current_element,
            html::ElementContent::Text(sanitized_string),
        );

        return srmarkup::ASTProcessorVisitResult::Continue;
    }

    fn visit_literal(
        &mut self,
        literal_node: &srmarkup::ASTNodeLiteral,
    ) -> srmarkup::ASTProcessorVisitResult {
        let current_element = *self.element_stack.last().unwrap();

        self.doc.push_content(
            current_element,
            html::ElementContent::Text(literal_node.to_string()),
        );

        return srmarkup::ASTProcessorVisitResult::Continue;
    }

    fn visit_end_tag(&mut self, _: &srmarkup::ASTNodeTag) {
        self.pop_element();
    }

    fn visit_end_root(&mut self, _: &srmarkup::ASTNodeRoot) {
        self.pop_element();
    }
}
