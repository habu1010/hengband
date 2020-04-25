#include "angband.h"
#include "autopick/autopick-util.h"
#include "autopick/autopick-reader-writer.h"
#include "files.h"

/*
 *  Get file name for autopick preference
 */
concptr pickpref_filename(player_type *player_ptr, int filename_mode)
{
	static const char namebase[] = _("picktype", "pickpref");

	switch (filename_mode)
	{
	case PT_DEFAULT:
		return format("%s.prf", namebase);

	case PT_WITH_PNAME:
		return format("%s-%s.prf", namebase, player_ptr->base_name);

	default:
		return NULL;
	}
}


/*
 * Read whole lines of a file to memory
 */
static concptr *read_text_lines(concptr filename)
{
	concptr *lines_list = NULL;
	FILE *fff;

	int lines = 0;
	char buf[1024];

	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, filename);
	fff = my_fopen(buf, "r");
	if (!fff) return NULL;

	C_MAKE(lines_list, MAX_LINES, concptr);
	while (my_fgets(fff, buf, sizeof(buf)) == 0)
	{
		lines_list[lines++] = string_make(buf);
		if (lines >= MAX_LINES - 1) break;
	}

	if (lines == 0)
		lines_list[0] = string_make("");

	my_fclose(fff);
	return lines_list;
}


/*
 * Add one line to autopick_list[]
 */
void add_autopick_list(autopick_type *entry)
{
	if (max_autopick >= max_max_autopick)
	{
		int old_max_max_autopick = max_max_autopick;
		autopick_type *old_autopick_list = autopick_list;
		max_max_autopick += MAX_AUTOPICK_DEFAULT;
		C_MAKE(autopick_list, max_max_autopick, autopick_type);
		(void)C_COPY(autopick_list, old_autopick_list, old_max_max_autopick, autopick_type);
		C_KILL(old_autopick_list, old_max_max_autopick, autopick_type);
	}

	autopick_list[max_autopick] = *entry;
	max_autopick++;
}


/*
 * Copy the default autopick file to the user directory
 */
static void prepare_default_pickpref(player_type *player_ptr)
{
	const concptr messages[] = {
		_("あなたは「自動拾いエディタ」を初めて起動しました。", "You have activated the Auto-Picker Editor for the first time."),
		_("自動拾いのユーザー設定ファイルがまだ書かれていないので、", "Since user pref file for autopick is not yet created,"),
		_("基本的な自動拾い設定ファイルをlib/pref/picktype.prfからコピーします。", "the default setting is loaded from lib/pref/pickpref.prf ."),
		NULL
	};

	concptr filename = pickpref_filename(player_ptr, PT_DEFAULT);
	for (int i = 0; messages[i]; i++)
	{
		msg_print(messages[i]);
	}

	msg_print(NULL);
	char buf[1024];
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, filename);
	FILE *user_fp;
	user_fp = my_fopen(buf, "w");
	if (!user_fp) return;

	fprintf(user_fp, "#***\n");
	for (int i = 0; messages[i]; i++)
	{
		fprintf(user_fp, "#***  %s\n", messages[i]);
	}

	fprintf(user_fp, "#***\n\n\n");
	path_build(buf, sizeof(buf), ANGBAND_DIR_PREF, filename);
	FILE *pref_fp;
	pref_fp = my_fopen(buf, "r");

	if (!pref_fp)
	{
		my_fclose(user_fp);
		return;
	}

	while (!my_fgets(pref_fp, buf, sizeof(buf)))
	{
		fprintf(user_fp, "%s\n", buf);
	}

	my_fclose(user_fp);
	my_fclose(pref_fp);
}

/*
 * Read an autopick prefence file to memory
 * Prepare default if no user file is found
 */
concptr *read_pickpref_text_lines(player_type *player_ptr, int *filename_mode_p)
{
	/* Try a filename with player name */
	*filename_mode_p = PT_WITH_PNAME;
	char buf[1024];
	strcpy(buf, pickpref_filename(player_ptr, *filename_mode_p));
	concptr *lines_list;
	lines_list = read_text_lines(buf);

	if (!lines_list)
	{
		*filename_mode_p = PT_DEFAULT;
		strcpy(buf, pickpref_filename(player_ptr, *filename_mode_p));
		lines_list = read_text_lines(buf);
	}

	if (!lines_list)
	{
		prepare_default_pickpref(player_ptr);
		lines_list = read_text_lines(buf);
	}

	if (!lines_list)
	{
		C_MAKE(lines_list, MAX_LINES, concptr);
		lines_list[0] = string_make("");
	}

	return lines_list;
}


/*
 * Write whole lines of memory to a file.
 */
bool write_text_lines(concptr filename, concptr *lines_list)
{
	char buf[1024];
	path_build(buf, sizeof(buf), ANGBAND_DIR_USER, filename);
	FILE *fff;
	fff = my_fopen(buf, "w");
	if (!fff) return FALSE;

	for (int lines = 0; lines_list[lines]; lines++)
	{
		my_fputs(fff, lines_list[lines], 1024);
	}

	my_fclose(fff);
	return TRUE;
}
