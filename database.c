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

void print_database(Database database);
char *create_return_string(Table *table);
char *get_fields(char ***fields, int *num_fields, const char *start);
char *execute_statement(Database *database, const char *statement);
char *select_statement(Database *database, const char *statement);
void free_database(Database *database);
char *get_field_value(char **field, const char *start, const char *delim);
void read_table(Table *table, const char *table_def, FILE *file);
int read_database(Database *database, const char *db_name);

int main(int argc, char *argv[])
{
	Database database;
	read_database(&database, "test_db.xml");
	print_database(database);
	printf("\n");

	char *line = NULL;
	size_t line_size = 0;
	int chars_read = getline(&line, &line_size, stdin);
	while (strcmp(line, "QUIT\n") != 0)
	{
		char *output = execute_statement(&database, line);
		printf("%s\n", output);
		free(output);
		chars_read = getline(&line, &line_size, stdin);
	}

	free(line);
	free_database(&database);

	return 0;
}

void print_database(Database database)
{
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
}

char *execute_statement(Database *database, const char *statement)
{	
	char *type;
	char *begin_next = get_field_value(&type, statement, " ");

	if (strcmp(type, "SELECT") == 0)
	{
		return select_statement(database, begin_next);
	}
	else
	{
		return strdup("Invalid statement.");
	}
}

char *select_statement(Database *database, const char *statement)
{
	char **fields = NULL;
	int num_fields = 0;
	char *begin_next = get_fields(&fields, &num_fields, statement);

	if ((begin_next == NULL) || (fields != NULL))
	{
		int i;
		for (i = 0; i < num_fields; i++)
		{
			printf("%s\n", fields[i]);
		}
		return strdup("Early return from select_statement\n");
	}

	char *from;
	begin_next = get_field_value(&from, begin_next, " ");
	if (strcmp(from, "FROM") != 0)
	{
		return strdup("That is not a valid SELECT statement.\n");
	}

	char *table, *where = NULL;
	char *temp = get_field_value(&table, begin_next, " ");
	//if there is no space, there must not be a WHERE clause,
	//so search instead for the end of the line to get the value for
	//table
	if (temp == NULL)
	{
		begin_next = get_field_value(&table, begin_next, "\n"); 
	}
	else
	{
		begin_next = get_field_value(&where, temp, "\n");
	}

	int i;
	for (i = 0; i < database->num_tables; i++)
	{
		if (strcmp(database->tables[i].name, table) == 0)
		{
			return create_return_string(&database->tables[i]);
		}
	}

	return strdup("That table does not exist.\n");
}

char *create_return_string(Table *table)
{
	char *return_string = (char *) malloc(1 * sizeof(char));
	*return_string = '\0';
	int curr_length = 1;
	int i;
	for (i = 0; i < table->num_fields; i++)
	{
		curr_length = strlen(table->fields[i].name) + curr_length + 2;
		//I realize this is terribly inefficient and should probably use the
		//convention of resizing the array to twice its size to accomodate the
		//new data, as needed; that optimization will be added later
		return_string = (char *) realloc(return_string, curr_length * sizeof(char));
		//just read today (though it shold probably have been obvious) on
		//joelonsoftware on how strcat scales awfully, because of its need to 
		//reach the end of the initial string each time; should probably
		//improve and optimize this later as well
		strcat(return_string, table->fields[i].name);
		strcat(return_string, ", ");
	}
	//return_string now has a single extra character
	return_string[curr_length - 3] = '\n';
	return_string[curr_length - 2] = '\0';
	for (i = 0; i < table->num_rows; i++)
	{
		int j;
		for (j = 0; j < table->num_fields; j++)
		{
			curr_length = strlen(table->rows[i].values[j]) + curr_length + 2;
			return_string = (char *) realloc(return_string, curr_length * sizeof(char));
			strcat(return_string, table->rows[i].values[j]);
			strcat(return_string, ", ");
		}
		return_string[strlen(return_string) - 2] = '\n';
		return_string[strlen(return_string) - 1] = '\0';
	}
	return return_string;
}

char *get_fields(char ***fields, int *num_fields, char const *start)
{
	if (start[0] == '*')
	{
		*fields = NULL;
		*num_fields = 0;
		return start + 2;
	}

	char *curr_field;
	char *begin_next = get_field_value(&curr_field, start, ",");
	
	if (begin_next == NULL)
	{
		return NULL;
	}

	(*num_fields)++;
	*fields = (char **) malloc(sizeof(char *));
	(*fields)[0] =	curr_field;

	int cont = 1;
	while (cont)
	{
		begin_next++;
		char *temp = get_field_value(&curr_field, begin_next, ",");
		if (temp != NULL)
		{
			begin_next = temp;
			(*num_fields)++;
			*fields = (char **) realloc(*fields, *num_fields * sizeof(char *));
			(*fields)[*num_fields - 1] = curr_field;
		}
		else
		{
			cont = 0;
		}
	}

	begin_next = get_field_value(&curr_field, begin_next, " ");
	if (begin_next == NULL)
	{
		return NULL;
	}

	(*num_fields)++;
	*fields = (char **) realloc(*fields, *num_fields * sizeof(char *));
	(*fields)[*num_fields - 1] = curr_field;

	return begin_next;
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
	free(database->name);
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
	//database->name = (char *) malloc((strlen(db_name) + 1) * sizeof(char));
	//strcpy(database->name, db_name);
	database->name = strdup(db_name);
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

	free(line);
	fclose(db_file);
}

//Gets the value 
char *get_field_value(char **field, const char *start, const char *delim)
{
	char *end = strstr(start, delim);
	if (end == NULL)
	{
		return NULL;
	}

	*end = '\0';
	*field = strdup(start);
	/*
	int length = strlen(start);
	*field = (char *) malloc((length + 1) * sizeof(char));
	strcpy(*field, start);*/
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
			//table->rows[table->num_rows - 1].values[i] = (char *) malloc(strlen(line) * sizeof(char));
			line[strlen(line) - 1] = '\0';
			table->rows[table->num_rows - 1].values[i] = strdup(line);
			//strcpy(table->rows[table->num_rows - 1].values[i], line);
		}
		//get rid of the current row's closing </row>
		chars_read = getline(&line, &line_size, file);
		//read the next line's <row> or </table>
		chars_read = getline(&line, &line_size, file);
	}
}
