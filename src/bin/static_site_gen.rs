use srmarkup;

use minijinja::{context, Environment};

static TEMPLATE_STR_HEAD: &str = r##"
<!doctype html>

<html lang="en">

<head>
  <meta charset="utf-8">

  <title>{{ title }}</title>
  <meta name="description" content="Shareef Raheem's portfolio and blog site.">
  <meta name="author" content="Shareef Raheem">
  <meta name="theme-color" content="#333">
  <meta name="viewport" content="width = device-width, initial-scale = 1.0" id="meta" />

  <link rel="shortcut icon" type="image/png" href="images/favicon.png" />

  <link rel="stylesheet" href="css/responsive.css">
  <link rel="stylesheet" href="css/styles.css">

  <script src="js/animation-lib.js" type="text/javascript"></script>
  <script src="js/blufedora.dom.js" type="text/javascript"></script>
  <script src="js/blufedora.urlvars.js" type="text/javascript"></script>
  <script src="js/blufedora.worker.js" type="text/javascript"></script>
  <script src="js/blufedora.blog.js" type="text/javascript"></script>
  <script src="https://cdn.jsdelivr.net/gh/google/code-prettify@master/loader/run_prettify.js?skin=desert"></script>

  <link rel="stylesheet" href="css/post-themes/base-theme.css">
</head>
"##;

static TEMPLATE_STR_BODY: &str = r#"
<body class="post-background">
  <section class="main-article" id="main_article">
    <a id="menu" href="blog.html">
      <div class="menu-bar"></div>
      <div class="menu-bar"></div>
      <div class="menu-bar"></div>
    </a>

    <div class="post-body">
      <div id="post-header">
        <div id="post-header-content">
        </div>
      </div>

      <div class="post-body-content"></div>

      <footer>
        <div class='credits'>
          <span style="color:#3c465f">Copyright &copy; 2014-2023</span>
          <span style="color:#51608b">Shareef Raheem</span>
        </div>
      </footer>
    </div>

  </section>
</body>

</html>
"#;

static TEMPLATE_STR: &str = r#"
{% include 'blog_header.html' %}
{% include 'blog_body.html' %}
"#;

struct BlogHeader {
    pub title: String,
    pub cover_image: String,
    pub date: String,
    pub theme: Option<String>,
}

pub mod html {
    use std::{collections::HashMap, default};

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

    impl Document {
        pub fn create_element(&mut self, tag: ElementTagID) -> ElementID {
            let element = Element {
                tag: tag,
                attributes: Default::default(),
                contents: Default::default(),
                is_comment: false,
            };
            let id = self.elements.len() as ElementID;
            self.elements.push(element);
            return id;
        }
    }

    impl Default for Document {
        fn default() -> Self {
            let mut result = Self {
              elements: Default::default(),
                #[rustfmt::skip]
                tags: vec![
                  // Resources:
                  //   - [Element Tag Listing](https://developer.mozilla.org/en-US/docs/Web/HTML/Element)
                  //   - [Void Elements](https://developer.mozilla.org/en-US/docs/Glossary/Void_element)

                  // Root Element
                  ElementTag{ start_tag: String::from("!doctype"), end_tag: Option::None, is_void_element: true},

                  // Document Metadata
                  ElementTag{ start_tag: String::from("base"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("head"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("link"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("meta"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("style"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("title"), end_tag: Option::None, is_void_element: false},

                  // Sectioning Root
                  ElementTag{ start_tag: String::from("body"), end_tag: Option::None, is_void_element: false},

                  // Content Sectioning
                  ElementTag{ start_tag: String::from("address"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("article"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("aside"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("footer"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("header"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("h1"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("h1"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("h2"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("h3"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("h4"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("h5"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("h6"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("nav"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("hgroup"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("section"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("search"), end_tag: Option::None, is_void_element: false},

                  // Text Content
                  ElementTag{ start_tag: String::from("blockquote"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("dd"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("div"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("dt"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("figcaption"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("figure"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("hr"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("li"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("menu"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("ol"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("p"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("pre"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("ul"), end_tag: Option::None, is_void_element: false},
                  
                  // Inline Text Semantics
                  ElementTag{ start_tag: String::from("a"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("abbr"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("b"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("bdi"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("bdo"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("br"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("cite"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("code"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("data"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("dfn"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("em"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("i"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("kbd"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("mark"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("q"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("rp"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("rt"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("ruby"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("s"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("samp"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("small"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("span"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("strong"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("sub"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("sup"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("time"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("u"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("var"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("wbr"), end_tag: Option::None, is_void_element: true},

                  // Image and Multimedia
                  ElementTag{ start_tag: String::from("area"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("audio"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("img"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("map"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("track"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("video"), end_tag: Option::None, is_void_element: false},

                  // Embedded Content
                  ElementTag{ start_tag: String::from("embed"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("iframe"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("object"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("picture"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("portal"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("source"), end_tag: Option::None, is_void_element: true},

                  // SVG and MathML
                  ElementTag{ start_tag: String::from("svg"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("math"), end_tag: Option::None, is_void_element: false},

                  // Scripting
                  ElementTag{ start_tag: String::from("canvas"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("noscript"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("script"), end_tag: Option::None, is_void_element: false},

                  // Demarcating Edits
                  ElementTag{ start_tag: String::from("del"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("ins"), end_tag: Option::None, is_void_element: false},

                  // Table Content
                  ElementTag{ start_tag: String::from("caption"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("col"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("colgroup"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("table"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("tbody"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("td"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("tfoot"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("th"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("thead"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("tr"), end_tag: Option::None, is_void_element: false},

                  // Forms
                  ElementTag{ start_tag: String::from("button"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("datalist"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("fieldset"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("form"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("input"), end_tag: Option::None, is_void_element: true},
                  ElementTag{ start_tag: String::from("label"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("legend"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("meter"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("optgroup"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("option"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("output"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("progress"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("select"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("textarea"), end_tag: Option::None, is_void_element: false},

                  // Interactive Elements
                  ElementTag{ start_tag: String::from("details"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("dialog"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("summary"), end_tag: Option::None, is_void_element: false},

                  // Web Components
                  ElementTag{ start_tag: String::from("slot"), end_tag: Option::None, is_void_element: false},
                  ElementTag{ start_tag: String::from("template"), end_tag: Option::None, is_void_element: false},
                ],
                doc_type: Default::default(),
                html: Default::default(),
                head: Default::default(),
                body: Default::default(),
            };

            result.tags.sort_unstable_by(|a: &ElementTag, b: &ElementTag| a.start_tag.partial_cmp(&b.start_tag).unwrap());

            return result;
        }
    }
}

fn main() {
    let doc: html::Document = Default::default();

    for tag in doc.tags.iter() {
        if tag.is_void_element {
            println!("<{}>", tag.start_tag);
        } else {
            match &tag.end_tag {
                Some(end_tag) => {
                    println!("<{}>...</{}>", tag.start_tag, end_tag);
                }
                None => {
                    println!("<{}>...</{}>", tag.start_tag, tag.start_tag);
                }
            }
        }
    }

    let mut env = Environment::new();
    env.add_template("blog_header.html", TEMPLATE_STR_HEAD)
        .unwrap();
    env.add_template("blog_body.html", TEMPLATE_STR_BODY)
        .unwrap();
    env.add_template("main_template", TEMPLATE_STR).unwrap();

    let main_template = env.get_template("main_template").unwrap();

    let ctx_vars = context! {
      title => "Test Title",
      description => "",
      theme => "",
    };

    println!("{}", main_template.render(ctx_vars).unwrap());

    // env.add_template("hello.txt", "Hello {{ name }}!").unwrap();
    // let template = env.get_template("hello.txt").unwrap();
    // println!("{}", template.render(context! { name => "World" }).unwrap());
}
