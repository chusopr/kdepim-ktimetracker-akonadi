/* libplugin.c
 *
 * Copyright (C) 2002 by Reinhold Kainhofer
 * Copyright (C) 1999 by Judd Montgomery
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "APIEmulation.h"
#include <string.h>




int jp_logf(int level, char *format, ...) {
	return jpilot_logf(level, format, va_args);

   printf("inside jp_logf (%s) \n",format);
	return 0;
}

int jpilot_logf(int level, char *format, ...) {
   printf("inside jpilot_logf (%s) \n",format);
	return 0;
}

/*creates the full path name of a file in the ~/.jpilot dir */
int get_home_file_name(char *file, char *full_name, int max_size)
{
   char *home, default_path[]=".";

   home = getenv("JPILOT_HOME");

   if (!home) {/*Not home; */
      home = getenv("HOME");
      if (!home) {/*Not home; */
	 jpilot_logf(LOG_WARN, "Can't get HOME environment variable\n");
      }
   }
   if (!home) {
      home = default_path;
   }
   if (strlen(home)>(max_size-strlen(file)-strlen("/.jpilot/")-2)) {
      jpilot_logf(LOG_WARN, "Your HOME environment variable is too long for me\n");
      home=default_path;
   }
   sprintf(full_name, "%s/.jpilot/%s", home, file);
   return 0;
}

FILE *jp_open_home_file(char *filename, char *mode)
{
   char fullname[256];
   FILE *pc_in;

   get_home_file_name(filename, fullname, 255);

   pc_in = fopen(fullname, mode);
   if (pc_in == NULL) {
      pc_in = fopen(fullname, "w+");
      if (pc_in) {
	 fclose(pc_in);
	 pc_in = fopen(fullname, mode);
      }
   }
   return pc_in;
}




/* Jason Day contributed code - Start */
/*
 * WARNING
 * Caller must ensure that which is not out of range!
 */
int jp_get_pref (prefType prefs[], int which, long *n, const char **ret)
{
    if (which < 0) {
        return -1;
    }
    *n = prefs[which].ivalue;
    if (prefs[which].usertype == CHARTYPE) {
        if (ret != NULL) {
            *ret = prefs[which].svalue;
        }
    }
    else {
        if (ret !=NULL) {
            *ret = NULL;
        }
    }
    return 0;
}

/*
 * WARNING
 * Caller must ensure that which is not out of range!
 */
int jp_set_pref (prefType prefs[], int which, long n, const char *string)
{
    if (which < 0) {
        return -1;
    }
    prefs[which].ivalue = n;
    if (string == NULL) {
        prefs[which].svalue[0] = '\0';
        return 0;
    }
    if (prefs[which].filetype == CHARTYPE) {
        strncpy (prefs[which].svalue, string, MAX_PREF_VALUE);
        prefs[which].svalue[MAX_PREF_VALUE - 1] = '\0';
    }
    return 0;
}

void jp_pref_init(prefType prefs[], int count)
{
   int i;

   for (i=0; i<count; i++) {
      if (prefs[i].svalue) {
	 prefs[i].svalue=strdup(prefs[i].svalue);
      } else {
	 prefs[i].svalue=strdup("");
      }
      prefs[i].svalue_size=strlen(prefs[i].svalue)+1;
   }
}

int jp_pref_read_rc_file(char *filename, prefType prefs[], int num_prefs)
{
   int i;
   FILE *in;
   char line[1024];
   char *field1, *field2;
   char *Pc;

   in=jp_open_home_file(filename, "r");
   if (!in) {
      return -1;
   }

   while (!feof(in)) {
      fgets(line, 1024, in);
      if (feof(in)) break;
      line[1023] = ' ';
      line[1023] = '\0';
      field1 = strtok(line, " ");
      field2 = (field1 != NULL)	? strtok(NULL, "\n") : NULL;/* jonh */
      if ((field1 == NULL) || (field2 == NULL)) {
	 continue;
      }
      if ((Pc = (char *)index(field2, '\n'))) {
	 Pc[0]='\0';
      }
      for(i=0; i<num_prefs; i++) {
	 if (!strcmp(prefs[i].name, field1)) {
	    if (prefs[i].filetype == INTTYPE) {
	       prefs[i].ivalue = atoi(field2);
	    }
	    if (prefs[i].filetype == CHARTYPE) {
	       if (pref_lstrncpy_realloc(&(prefs[i].svalue), field2,
					&(prefs[i].svalue_size),
					MAX_PREF_VALUE)==NULL) {
		  jpilot_logf(LOG_WARN, "Out of memory: read_rc_file()\n");
		  continue;
	       }
	    }
	 }
      }
   }
   fclose(in);

   return 0;
}

int jp_pref_write_rc_file(char *filename, prefType prefs[], int num_prefs)
{
   int i;
   FILE *out;


   out=jp_open_home_file(filename,"w" );
   if (!out) {
      return -1;
   }

   for(i=0; i<num_prefs; i++) {

      if (prefs[i].filetype == INTTYPE) {
	 fprintf(out, "%s %ld\n", prefs[i].name, prefs[i].ivalue);
      }

      if (prefs[i].filetype == CHARTYPE) {
	 fprintf(out, "%s %s\n", prefs[i].name, prefs[i].svalue);
      }
   }


   fclose(out);

   return 0;
}

char *pref_lstrncpy_realloc(char **dest, const char *src, int *size, int max_size)
{
   int new_size, len;
   const char null_str[]="";
   const char *Psrc;

   if (!src) {
      Psrc=null_str;
   } else {
      Psrc=src;
   }
   len=strlen(Psrc)+1;
   new_size=*size;
   if (len > *size) {
      new_size=len;
   }
   if (new_size > max_size) new_size=max_size;

   if (new_size > *size) {
      if (*size == 0) {
	 *dest=malloc(new_size);
      } else {
	 *dest=realloc(*dest, new_size);
      }
      if (!(*dest)) {
	 return 0;
      }
      *size=new_size;
   }
   strncpy(*dest, Psrc, new_size);
   (*dest)[new_size-1]='\0';

   return *dest;
}

void jp_charset_j2p(unsigned char *const buf, int max_len)
{
   long char_set;

   get_pref(PREF_CHAR_SET, &char_set, NULL);
   charset_j2p(buf, max_len, char_set);
}

void jp_charset_p2j(unsigned char *const buf, int max_len)
{
   long char_set;

   get_pref(PREF_CHAR_SET, &char_set, NULL);
   if (char_set == CHAR_SET_JAPANESE) jp_Sjis2Euc(buf, max_len);
   if (char_set == CHAR_SET_1250) Win2Lat(buf,max_len);
   if (char_set == CHAR_SET_1251) win1251_to_koi8(buf, max_len);
   if (char_set == CHAR_SET_1251_B) koi8_to_win1251(buf, max_len);
}


/*
 * Widget must be some widget used to get the main window from.
 * The main window passed in would be fastest.
 * changed is MODIFY_FLAG, or NEW_FLAG
 */
int dialog_save_changed_record(GtkWidget *widget, int changed)
{
   GtkWidget *w;
   int i, b;
   char *button_text[]={gettext_noop("Yes"), gettext_noop("No")};

   b=0;

   if ((changed!=MODIFY_FLAG) && (changed!=NEW_FLAG)) {
      return 0;
   }
   /* Find the main window from some global widget and do a dialog */
   for (w=widget, i=10; w && (i>0); w=w->parent, i--) {
      if (GTK_IS_WINDOW(w)) {
	 if (changed==MODIFY_FLAG) {
	    b=dialog_generic(GTK_WIDGET(w)->window, 0, 0,
			     _("Save Changed Record?"), "",
			     _("Do you want to save the changes to this record?"),
			     2, button_text);
	 }
	 if (changed==NEW_FLAG) {
	    b=dialog_generic(GTK_WIDGET(w)->window, 0, 0,
			     _("Save New Record?"), "",
			     _("Do you want to save this new record?"),
			     2, button_text);
	 }
	 break;
      }
   }
   return b;
}
