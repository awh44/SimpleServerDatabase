#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

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
void read_table(Table *table, const char *table_def, FILE *file);
int read_database(Database *database, const char *db_name);

int main(int argc, char *argv[])
{
	Database database;
	database.name = NULL;
	database.num_tables = 0;
	database.tables = NULL;

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

	chars_read = getline(&line, &line_size, db_file);
	while (strcmp(line, "</database>\n") != 0)
	{
		printf("%s", line);
		database->num_tables++;
		database->tables = realloc(database->tables, database->num_tables * sizeof(Table));
		read_table(&database->tables[database->num_tables - 1], db_file);
		chars_read = getline(&line, &line_size, db_file);
	}	

	fclose(db_file);
}

void read_table(Table *table, FILE *file)
{
	
}

void read_row(Field *field, FILE *file)
{
	
}
