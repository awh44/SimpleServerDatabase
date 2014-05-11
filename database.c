#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define NAME_START 14
#define BUFFER_SIZE 64

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
	Row *rows;
} Table;

typedef struct
{
	char *name;
	int num_tables;
	Table *tables;
} Database;

void read_row(Field *field, FILE *file);
char *get_field_value(char **field, const char *start, const char *delim);
void read_table(Table *table, const char *table_def, FILE *file);
int read_database(Database *database, const char *db_name);

int main(int argc, char *argv[])
{
	Database database;
	read_database(&database, "test_db.xml");

	return 0;
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
		printf("%s", line);
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
	*end = delim[0];
	return end + 1;
}

void read_table(Table *table, const char *table_def, FILE *file)
{
	//initialize table
	table->num_fields = 0;
	table->fields = NULL;
	table->rows = NULL;

	char *next_start = get_field_value(&table->name, &table_def[NAME_START], "\"");
	while (*next_start != '>')
	{
		table->num_fields++;
		table->fields = realloc(table->fields, table->num_fields * sizeof(Field));
		next_start = get_field_value(&table->fields[table->num_fields - 1].name, next_start + 1, "=");
		printf("%s\n", table->fields[table->num_fields - 1].name);
		next_start = get_field_value(&table->fields[table->num_fields - 1].type, next_start + 1, "\"");
		printf("%s\n", table->fields[table->num_fields - 1].type);
	}

	char *line = NULL;
	size_t line_size = 0;
	int chars_read = getline(&line, &line_size, file);
	while (strcmp(line, "\t</table>\n") != 0)
	{
		getline(&line, &line_size, file);
	}
}

void read_row(Field *field, FILE *file)
{
	
}
