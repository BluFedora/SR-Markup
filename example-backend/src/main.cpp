//
// Shareef Abdoul-Raheem
//
#include "sr-markup.h"

#include "bf/utility/bifrost_json.hpp"

#include <iostream>
#include <string>

static bool MatchString(StringView lhs, const char* rhs)
{
  while (lhs.str_start != lhs.str_end  && *rhs)
  {
    if (*lhs.str_start != *rhs)
    {
      return false;
    }

    ++lhs.str_start;
    ++rhs; 
  }

  return lhs.str_start == lhs.str_end && *rhs == '\0';
}

static StringView StringViewFromCStr(const char* string)
{
  return StringView{
    string,
    string + std::strlen(string)
  };
}

static const char* TagTypeToStr(ASTNodeLiteralValue_Tag type)
{
  switch (type)
  {
    case AsStr:     return "String";
    case AsNumber:  return "Number";
    case AsBoolean: return "Boolean";
  }

  return "__ERROR__";
}

#define STR_PRINT(str) int((str).str_end - (str).str_start), (str).str_start

static const ASTNodeLiteralValue* FindAttribute(const char* key, const ASTNode_TagNode_Body& tag_node, ASTNodeLiteralValue_Tag type, bool required = true)
{
  const TagAttributeView* attribs     = tag_node.attributes;
  const uint32_t          num_attribs = tag_node.num_attributes;

  for (uint32_t i = 0u; i < num_attribs; ++i)
  {
    if (MatchString(attribs[i].key, key))
    {
      if (attribs[i].value.tag != type)
      {
        std::fprintf(stderr,
          "[ERROR]: Found Attribute(%s) but it was of type %s rather than %s.\n",
          key,
          TagTypeToStr(attribs[i].value.tag),
          TagTypeToStr(type));
        
        return nullptr;
      }

      return &attribs[i].value;
    }
  }

  if (required)
  {
   std::fprintf(stderr,
    "[ERROR]: Failed to find Attribute(%s) in TagNodeBody(%.*s).\n",
    key,
    STR_PRINT(tag_node.text)
    );
  }

  return nullptr;
}

StringView ASTNodeLiteralValueString(const ASTNodeLiteralValue* node)
{
  switch (node->tag)
  {
    case AsStr:
    {
      return node->as_str.value;
    }
    case AsNumber:
    {
      return StringViewFromCStr("Number");
    }
    case AsBoolean:
    {
      return node->as_boolean.value ? StringViewFromCStr("true") : StringViewFromCStr("false");
    }
  }

  return {nullptr, nullptr};
}

using namespace bf::json;

#pragma comment(lib, "lib/BF_DataStructuresC_static.lib")
#pragma comment(lib, "lib/BF_JsonCpp.lib")

struct Allocator : public bf::IMemoryManager
{
  void* allocate(std::size_t size) override{ return malloc(size); }
  void deallocate(void* ptr, std::size_t) override { free(ptr); }
};

static Value s_RootJson    = Object{};
static Value s_ContentJson = {};
static Allocator s_A = {};

static void RecursivelyProcessNode(const ASTNode* parent_node, Value& parent_json_value)
{
  switch (parent_node->tag)
  {
    case ASTNode_TagNode:
    {
      const auto& tag_node = parent_node->tag_node;

      if (MatchString(tag_node.text, "Header"))
      {
        const auto* title     = FindAttribute("Title", tag_node, AsStr);
        const auto* cover_img = FindAttribute("CoverImage", tag_node, AsStr);
        const auto* date      = FindAttribute("Date", tag_node, AsStr);
        const auto* theme     = FindAttribute("Theme", tag_node, AsStr);
        
        if (title && cover_img && date)
        {
          const auto& title_str_value = title->as_str.value;
          const auto& image_str_value = cover_img->as_str.value;
          const auto& date_str_value  = date->as_str.value;

          s_RootJson["Title"] = String(title_str_value.str_start, title_str_value.str_end);

          s_RootJson["Header"] = {
            Pair{"Image",  String(image_str_value.str_start, image_str_value.str_end)},
            Pair{"Author", String("By: Shareef Raheem") },
            Pair{"Date",   String(date_str_value.str_start, date_str_value.str_end) },
          };

          if (theme)
          {
            s_RootJson["Header"]["Theme"] = String(theme->as_str.value.str_start, theme->as_str.value.str_end);
          }
        }

        return;
      }
      
      String css_classes           = "";
      const auto* css_classes_attr = FindAttribute("Class", tag_node, AsStr, false);
      const auto* css_size         = FindAttribute("Size", tag_node, AsStr, false);
      const auto* css_id           = FindAttribute("ID", tag_node, AsStr, false);
      
      if (css_classes_attr)
      {
        css_classes.append(bf::StringRange(css_classes_attr->as_str.value.str_start, css_classes_attr->as_str.value.str_end));
      }

      if (css_size)
      {
        if (MatchString(css_size->as_str.value, "Full"))
        {
          css_classes.append(bf::StringRange(" post-full"));
        }
        else if (MatchString(css_size->as_str.value, "Half"))
        {
          css_classes.append(bf::StringRange(" post-half"));
        }
        else
        {
          std::fprintf(stderr, "[ERROR]: Unknown Size %.*s (Must be either [\"Full\" or \"Half\"]).\n", STR_PRINT(css_size->as_str.value));
        }
      }

      if (MatchString(tag_node.text, "Text") || MatchString(tag_node.text, "text"))
      {
        parent_json_value["Type"] = "p";
      }
      else if (MatchString(tag_node.text, "Image") || MatchString(tag_node.text, "image"))
      {
        parent_json_value["Type"] = "img";

        const auto* img_source = FindAttribute("Src", tag_node, AsStr);

        if (img_source)
        {
          parent_json_value["Source"] = String(img_source->as_str.value.str_start, img_source->as_str.value.str_end);
        }
      }
      else if (MatchString(tag_node.text, "Link") || MatchString(tag_node.text, "link"))
      {
        parent_json_value["Type"] = "a";

        const auto* link_source = FindAttribute("Src", tag_node, AsStr);

        if (link_source)
        {
          parent_json_value["Source"] = String(link_source->as_str.value.str_start, link_source->as_str.value.str_end);
        }
      }
      else if (MatchString(tag_node.text, "UList") || MatchString(tag_node.text, "ulist"))
      {
        parent_json_value["Type"] = "ul";
        
      }
      else if (MatchString(tag_node.text, "OList") || MatchString(tag_node.text, "olist"))
      {
        parent_json_value["Type"] = "ol";
        
      }
      else if (MatchString(tag_node.text, "ListItem") || MatchString(tag_node.text, "listitem"))
      {
        parent_json_value["Type"] = "li";
      }
      else if (MatchString(tag_node.text, "Video") || MatchString(tag_node.text, "video"))
      {
        parent_json_value["Type"] = "video";

        const auto* video_source = FindAttribute("Src", tag_node, AsStr);

        if (video_source)
        {
          parent_json_value["Source"] = String(video_source->as_str.value.str_start, video_source->as_str.value.str_end);
        }
      }
      else
      {
        parent_json_value["Type"] = String(tag_node.text.str_start, tag_node.text.str_end);
      }

      Value content = {};

      for (uint32_t i = 0; i < tag_node.children.num_nodes; ++i)
      {
        const ASTNode& child_node = tag_node.children.nodes[i];
        Value child_node_as_json = {};

        RecursivelyProcessNode(&child_node, child_node_as_json);

        if (child_node_as_json.valid())
        {
          content.push(child_node_as_json);
        }
      }

      if (content.valid())
      {
        parent_json_value["Content"] = content;
      }

      if (!css_classes.isEmpty())
      {
        parent_json_value["Class"] = css_classes;
      }

      if (css_id)
      {
        parent_json_value["ID"] = String(css_id->as_str.value.str_start, css_id->as_str.value.str_end);
      }
      break;
    }
    case ASTNode_TextNode:
    {
      const auto text_str = parent_node->text_node.text;

      parent_json_value = String(text_str.str_start, text_str.str_end);
      break;
    }
    case ASTNode_LiteralNode:
    {
      const auto& lit_node = parent_node->literal_node.value;

      switch (lit_node.tag)
      {
        case AsStr:
        {
          parent_json_value = String("True");
          break;
        }
        case AsNumber:
        {
          std::string str = std::to_string(lit_node.as_number.value);
          str.erase ( str.find_last_not_of('0') + 1, std::string::npos); // Remove trailing 0s

          // If there is trailing '.' remove it.
          if (!str.empty() && str.back() == '.')
          {
            str.pop_back();
          }

          parent_json_value = String(str.c_str());
          break;
        }
        case AsBoolean:
        {
          parent_json_value = String(lit_node.as_boolean.value ? "true" : "false");
          break;
        }
      }
      break;
    }
  }
}

extern "C" 
{
  __declspec(dllexport) void* srMarkupInit(const Arguments* args)
  {
    /*
    for (uint32_t i = 0; i < args->num_args; ++i)
    {
      std::printf("Arg(%i) = %.*s\n", int(i), STR_PRINT(args->args[i]));
    }
    */
   (void)args;
    return nullptr;
  }
 
  __declspec(dllexport) void srMarkupProcess(const ASTNode* node, void* user_data)
  {
    Value node_as_json;
    RecursivelyProcessNode(node, node_as_json);

    if (node_as_json.valid())
    {
      s_ContentJson.push(node_as_json);
    }
  }

  __declspec(dllexport) void srMarkupShutdown(void* user_data)
  {
    //s_ContentJson.cast<Array>(s_A);
    s_RootJson["Content"] = s_ContentJson;

    String result_string;
    toString(s_RootJson, result_string);

    std::printf("%s", result_string.c_str());
  }
}