/* 
 * Copyright (C) 2013  Inori Sakura <inorindesu@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/*
  This application demostrate how to use pango's itemize function.
  After itemize, texts from different scripts will be separated, which
  will be convenient for rendering etc.
 */
#include <pango/pango.h>
#include <pango/pangoft2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE (1024)

void print_item(PangoItem* item, gpointer* userData)
{
  char* originalText = (char*)userData;
  
  printf("Offset: %d\n", item->offset);
  printf("Length: %d\n", item->length);
  printf("#chars: %d\n", item->num_chars);

  char* buffer = calloc(item->length + 1, 1);
  g_utf8_strncpy(buffer, originalText + item->offset, item->num_chars);
  printf("Text: %s\n", buffer);
  free(buffer);

  printf("Analysis:\n");
  PangoFontDescription* desc = pango_font_describe(item->analysis.font);
  printf("\tFont: %s\n", pango_font_description_get_family(desc));
  pango_font_description_free(desc);

  /*
    pango_language_to_string were blocked...
    ( As a macro in header pango-language.h ... :( )

    printf("\tLanguage: %s\n", pango_language_to_string(item->analysis.language));
    
    So I used script information.
    
    To add more, you may consult pango documentation
    https://developer.gnome.org/pango/stable/pango-Scripts-and-Languages.html
  */
  printf("\tScript ID: %d\n", item->analysis.script);
  char* scriptName = "NOT CLASSIFIED (too lazy)";
  if (item->analysis.script == PANGO_SCRIPT_TIBETAN)
    scriptName = "Tibetan";
  else if (item->analysis.script == PANGO_SCRIPT_LATIN)
    scriptName = "Latin";
  else if (item->analysis.script == PANGO_SCRIPT_HAN)
    scriptName = "Han (traditional & simpified)";
  else if (item->analysis.script == PANGO_SCRIPT_KATAKANA)
    scriptName = "Katakana";
  else if (item->analysis.script == PANGO_SCRIPT_HIRAGANA)
    scriptName = "Hiragana";
  else if (item->analysis.script == PANGO_SCRIPT_ARABIC)
    scriptName = "Arabic";
  printf("\tScript is %s script!\n", scriptName);
  printf("\n");
}

void print_header()
{
}

int main()
{
  PangoFontMap* fontMap = pango_ft2_font_map_new();
  PangoContext* context = pango_font_map_create_context(fontMap);

  /* content length of the buffer */
  int length = 0;
  char* buf = (char*)calloc(BUFSIZE, 1);
  int exitWhenDone = 0;
  while(1)
    {
      fprintf(stderr, "CC length %d\n", length);
      /*
        Load texts from stdin
      */
      int targetReadCount = BUFSIZE - length;
      int readCount = fread(&buf[length], 1, targetReadCount, stdin);
      if (readCount != targetReadCount)
        {
          if (feof(stdin))
            {
              exitWhenDone = 1;
            }
          else
            {
              fprintf(stderr, "ERROR: when reading stdin\n");
              break;
            }
        }
      length += readCount;

      /*
        Calculate unicode text length
       */
      int uniLength = g_utf8_strlen(buf, length);
      if (uniLength == 0)
        {
          fprintf(stderr, "ERROR: no utf-8 text in stream.\n");
          break;
        }
      char* boundary = g_utf8_offset_to_pointer(buf, uniLength);
      
      /*
        Itemize them
       */
      PangoAttrList* attrList = pango_attr_list_new();
      GList* items = pango_itemize_with_base_dir(context,
                                                 PANGO_DIRECTION_LTR,
                                                 buf,
                                                 0,
                                                 (boundary - buf),
                                                 attrList,
                                                 NULL);
      /*
        Output them
       */
      print_header();
      g_list_foreach(items, (GFunc)print_item, buf);
      
      /*
        recycle junks produced while itemizing
       */
      g_list_free_full(items, (GDestroyNotify)pango_item_free);
      pango_attr_list_unref(attrList);

      if (exitWhenDone) break;

      /*
        arrange buffer
       */
      int copyLength = (buf + BUFSIZE) - boundary;
      if (copyLength != 0)
        {
          /* dest, src*/
          memmove(buf, boundary, copyLength);
          length = copyLength;
        }
      else
        {
          length = 0;
        }
    }
  
  free(buf);
  g_object_unref(fontMap);
  g_object_unref(context);
  return 0;
}
