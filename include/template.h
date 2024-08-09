#pragma once 

/* Structure representing a template with a title, commands, and limits */
typedef struct {
    char* title;          /* Name of the template */
    char** arrayCommands;  /* Array of commands */
    int size;      /* Total number of commands */
} Template;

/* Structure representing a collection of templates */
typedef struct {
    Template** arrayTemplates;  /* Array of template pointers */
    int size;       /* Total number of templates */
} Templates;

/* Creates a new template with the given title */
Template* createTemplate(const char* title);

/* Deletes a template, freeing its memory */
void removeTemplate(Template* template);

/* Includes a origin into the template */
void insertTemplate(Template* template, const char* origin);

/* Creates a new collection of templates */
Templates* createTemplates();

/* Deletes a collection of templates, freeing its memory */
void removeTemplates(Templates* templates);

/* Searches for a template by title in the collection */
Template* findInTemplates(Templates* templates, const char* searching);

/* Records a template with the given title in the collection */
Template* recordTemplates(Templates* templates, const char* title);

