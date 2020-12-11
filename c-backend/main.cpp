
#include "sr-blog-gen.h"

#include "bifrost/utility/bifrost_json.h"

#include <iostream>

static bool MatchString(const StringView& lhs, const char* rhs)
{
  return std::strncmp(lhs.str_start, rhs, lhs.str_end - lhs.str_start) == 0;
}

static StringView StringViewFromCStr(const char* string)
{
  return StringView{
    string,
    string + std::strlen(string)
  };
}

#define STR_PRINT(str) int((str).str_end - (str).str_start), (str).str_start

static const ASTNodeLiteralValue* FindAttribute(const char* key, const ASTNode_TagNode_Body& tag_node)
{
  const TagAttributeView* attribs     = tag_node.attributes;
  const uint32_t          num_attribs = tag_node.num_attributes;

  for (uint32_t i = 0u; i < num_attribs; ++i)
  {
    if (MatchString(attribs[i].key, key))
    {
      return &attribs[i].value;
    }
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
}

static bfJsonWriter* s_JWriter;

extern "C" 
{
  __declspec(dllexport) void* srBlogGenInit(void)
  {
    s_JWriter =bfJsonWriter_newCRTAlloc();

    bfJsonWriter_beginObject(s_JWriter);
    return nullptr;
  }
 
  __declspec(dllexport) void srBlogGenProcess(const ASTNode* node, void* user_data)
  {
    switch (node->tag)
    {
      case ASTNode_TagNode:
      {
        if (MatchString(node->tag_node.text, "Header"))
        {
          const auto* title     = FindAttribute("Title", node->tag_node);
          const auto* cover_img = FindAttribute("CoverImage", node->tag_node);

          if (title)
          {
            if (title->tag == AsStr)
            {
              const auto& str_value = title->as_str.value;

              bfJsonWriter_key(s_JWriter, bfJsonString_fromCstr("Title"));
              bfJsonWriter_valueString(s_JWriter, bfJsonString_fromRange(str_value.str_start, str_value.str_end));
              bfJsonWriter_next(s_JWriter);
            }
            else
            {
              std::fprintf(stderr , "The Title is not a string.");
            }
          }
          else
          {
            std::fprintf(stderr , "Failed to Find Title Tag In The Header.");
          }

          if (cover_img && cover_img->tag == AsStr)
          {
            const auto& str_value = title->as_str.value;

              bfJsonWriter_key(s_JWriter, bfJsonString_fromCstr("Header"));
              bfJsonWriter_beginObject(s_JWriter);

              bfJsonWriter_key(s_JWriter, bfJsonString_fromCstr("Image"));
              bfJsonWriter_valueString(s_JWriter, bfJsonString_fromRange(str_value.str_start, str_value.str_end));
              bfJsonWriter_next(s_JWriter);

              bfJsonWriter_key(s_JWriter, bfJsonString_fromCstr("Author"));
              bfJsonWriter_valueString(s_JWriter, bfJsonString_fromCstr("By: Shareef Raheem"));
              bfJsonWriter_next(s_JWriter);

              bfJsonWriter_key(s_JWriter, bfJsonString_fromCstr("Date"));
              bfJsonWriter_valueString(s_JWriter, bfJsonString_fromCstr("Month Day, Year"));
              bfJsonWriter_next(s_JWriter);

              bfJsonWriter_endObject(s_JWriter);
              bfJsonWriter_next(s_JWriter);
          }
        }
        else if (MatchString(node->tag_node.text, "p"))
        {
          
        }
        else
        {
           printf(
           "TAG Node(%.*s)\n", 
           int(node->tag_node.text.str_end - node->tag_node.text.str_start), 
           node->tag_node.text.str_start);

          const TagAttributeView* attribs     = node->tag_node.attributes;
          const uint32_t          num_attribs = node->tag_node.num_attributes;

          for (uint32_t i = 0u; i < num_attribs; ++i)
          {
            auto val_as_str = ASTNodeLiteralValueString(&attribs[i].value);

            printf("  Attrib(%.*s, %.*s)\n", STR_PRINT(attribs[i].key), STR_PRINT(val_as_str));
          }
        }

        break;
      }
      case ASTNode_TextNode:
      {
        printf(
          "Text Node(%.*s)\n", 
          int(node->text_node.text.str_end - node->text_node.text.str_start), 
          node->text_node.text.str_start);
        break;
      }
      case ASTNode_LiteralNode:
      {
        printf("Literal Node\n");
        break;
      }
    };
  }

  __declspec(dllexport) void srBlogGenShutdown(void* user_data)
  {
    bfJsonWriter_endObject(s_JWriter);

    bfJsonWriter_forEachBlock(s_JWriter, [](const bfJsonStringBlock* block, void* user_data) {
      const auto as_str = bfJsonStringBlock_string(block);
      
      std::printf("%.*s", int(as_str.length), as_str.string);

    }, nullptr);

    bfJsonWriter_deleteCRT(s_JWriter);
  }
}