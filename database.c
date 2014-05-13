#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NAME_START 14
#define NUM_TABS 3 

typedef struct
{
	char *name;
	char *type;
} Field;

typedef struct
{
	char **values;
} Row;

typedef struct
{
	char *name;
	int num_fields;
	Field *fields;
	int num_rows;
	Row *rows;
} Table;

typedef struct
{
	char *name;
	int num_tables;
	Table *tables;
} Database;

char *read_fields(char ***fields, int *num_fields, const char *start);
void execute_statement(Database *database, const char *statement);
void select_statement(Database *database, const char *statement);
void free_database(Database *database);
char *get_field_value(char **field, const char *start, const char *delim);
void read_table(Table *table, const char *table_def, FILE *file);
int read_database(Database *database, const char *db_name);

int main(int argc, char *argv[])
{
	Database database;
	read_database(&database, "test_db.xml");

	/*
	int i;
	for (i = 0; i < database.num_tables; i++)
	{
		printf("Table: %s\n", database.tables[i].name);
		int j;
		for (j = 0; j < database.tables[i].num_rows; j++)
		{
			printf("\tRow %d:\n", j);
			int k;
			for (k = 0; k < database.tables[i].num_fields; k++)
			{
				printf("\t\t%s = %s", database.tables[i].fields[k].name, database.tables[i].rows[j].values[k]);
				printf(" (Type %s)\n", database.tables[i].fields[k].type);
			}
		}
	}
	*/

	char *line = NULL;
	size_t line_size = 0;
	int chars_read = getline(&line, &line_size, stdin);
	execute_statement(&database, line);

	free_database(&database);

	return 0;
}

void execute_statement(Database *database, const char *statement)
{
	
	char *type;
	char *begin_next = get_field_value(&type, statement, " ");

	if (strcmp(type, "SELECT") == 0)
	{
		select_statement(database, begin_next);
	}
}

void select_statement(Database *database, const char *statement)
{
	char **fields = NULL;
	int num_fields = 0;
	char *begin_next = read_fields(&fields, &num_fields, statement);
	
	if (begin_next == NULL)
		return;
	
	printf("%s", begin_next);
	char *from;
	begin_next = get_field_value(&from, begin_next, " ");
	//if (
}

char *read_fields(char ***fields, int *num_fields, const char *start)
{
	if (start[0] == '*')
	{
		*fields = NULL;
		*num_fields = 0;
		return start + 2;
	}
}

void free_database(Database *database)
{
	int i;
	for (i = 0; i < database->num_tables; i++)
	{
		free(database->tables[i].name);
		int j;
		for (j = 0; j < database->tables[i].num_fields; j++)
		{
			free(database->tables[i].fields[j].name);
			free(database->tables[i].fields[j].type);
		}
		free(database->tables[i].fields);

		for (j = 0; j < database->tables[i].num_rows; j++)
		{
			int k;
			for (k = 0; k < database->tables[i].num_fields; k++)
			{
				free(database->tables[i].rows[j].values[k]);
			}
			free(database->tables[i].rows[j].values);
		}
		free(database->tables[i].rows);
	}
	free(database->tables);
}

int read_database(Database *database, const char *db_name)
{
	FILE *db_file = fopen(db_name, "r");
	if (db_file == NULL)
	{
		printf("Couldn't open the database.\n");
		return 0;
	}

	char *line = NULL;
	size_t line_size = 0;
	int chars_read = getline(&line, &line_size, db_file);
	if (strcmp(line, "<database>\n") != 0)
	{
		printf("Specified file is not a compatible database.\n");
		fclose(db_file);
		return 0;
	}

	//now that it is known that failure won't occur, assign the
	//database values
	database->name = (char *) malloc((strlen(db_name) + 1) * sizeof(char));
	strcpy(database->name, db_name);
	database->num_tables = 0;
	database->tables = NULL;

	chars_read = getline(&line, &line_size, db_file);
	while (strcmp(line, "</database>\n") != 0)
	{
		database->num_tables++;
		database->tables = realloc(database->tables, database->num_tables * sizeof(Table));
		read_table(&database->tables[database->num_tables - 1], line, db_file);
		chars_read = getline(&line, &line_size, db_file);
	}	

	fclose(db_file);
}

char *get_field_value(char **field, const char *start, const char *delim)
{
	char *end = strstr(start, delim);
	*end = '\0';
	int length = strlen(start);
	*field = (char *) malloc((length + 1) * sizeof(char));
	strcpy(*field, start);
	*end = *delim;
	return end + 1;
}

void read_table(Table *table, const char *table_def, FILE *file)
{
	//initialize table
	table->num_fields = 0;
	table->fields = NULL;
	table->num_rows = 0;
	table->rows = NULL;

	char *next_start = get_field_value(&table->name, &table_def[NAME_START], "\"");
	while (*next_start != '>')
	{
		table->num_fields++;
		table->fields = realloc(table->fields, table->num_fields * sizeof(Field));
		next_start = get_field_value(&table->fields[table->num_fields - 1].name, next_start + 1, "=");
		next_start = get_field_value(&table->fields[table->num_fields - 1].type, next_start + 1, "\"");
	}

	char *line = NULL;
	size_t line_size = 0;
	int chars_read = getline(&line, &line_size, file);
	while (strcmp(line, "\t</table>\n") != 0)
	{	
		table->num_rows++;
		table->rows = realloc(table->rows, table->num_rows * sizeof(Row));
		table->rows[table->num_rows - 1].values = malloc(table->num_fields * sizeof(char *));
		int i;
		for (i = 0; i < table->num_fields; i++)
		{
			chars_read = getline(&line, &line_size, file);
			line += NUM_TABS;
			table->rows[table->num_rows - 1].values[i] = (char *) malloc(strlen(line) * sizeof(char));
			line[strlen(line) - 1] = '\0';
			strcpy(table->rows[table->num_rows - 1].values[i], line);
		}
		//get rid of the current row's closing </row>
		chars_read = getline(&line, &line_size, file);
		//read the next line's <row> or </table>
		chars_read = getline(&line, &line_size, file);
	}
}
