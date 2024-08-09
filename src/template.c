#include <stdlib.h>
#include <string.h>

#include "extra.h"
#include "template.h"

/* Allocate memory for a new Template structure and initialize its fields */
Template* createTemplate(const char* title) {
    Template* template;

    template = malloc(sizeof(*template)); /* Allocate memory for the template structure */

    template->title = copyString(title); /* Clone and store the title */
    template->arrayCommands = NULL; /* Initialize arrayCommands */
    template->size = 0; /* Initialize size */

    return template; /* Return the initialized template structure */
}

/* Free memory allocated for a Template structure and its fields */
void removeTemplate(Template* template) {
    int i;

    free(template->title); /* Free memory allocated for title */

    /* Free memory allocated for arrayCommands */
    for (i = 0; i < template->size; i++) {
        free(template->arrayCommands[i]);
    }

    free(template->arrayCommands); /* Free arrayCommands array */
    free(template); /* Free memory allocated for the template structure */
}

/* Include a string in the Template's arrayCommands */
void insertTemplate(Template* current, const char* origin) {
    char* copied;
    current->size++; /* Increment size */
    /* Reallocate memory for arrayCommands */
    current->arrayCommands = realloc(
        current->arrayCommands,
        sizeof(*current->arrayCommands) * current->size
    );

    copied = copyString(origin); /* Clone the string */
    
    /* Add the cloned string to arrayCommands */
    current->arrayCommands[current->size - 1] = copied;
}

/* Allocate memory for a new Templates structure and initialize its fields */
Templates* createTemplates() {
    Templates* result;
    result = malloc(sizeof(*result)); /* Allocate memory for the result structure */
    result->arrayTemplates = NULL; /* Initialize arrayTemplates */
    result->size = 0; /* Initialize size */

    return result; /* Return the initialized result structure */
}

/* Free memory allocated for a Templates structure and its fields */
void removeTemplates(Templates* current) {
    int i;

    /* Free memory allocated for each template in arrayTemplates */
    for (i = 0; i < current->size; i++) {
        removeTemplate(current->arrayTemplates[i]);
    }
    free(current->arrayTemplates); /* Free arrayTemplates array */
    free(current); /* Free memory allocated for the templates structure */
}

/* Search for a Template by title within Templates */
Template* findInTemplates(Templates* current, const char* searching) {
    int i;
    
    /* Iterate through arrayTemplates to find a matching title */
    for (i = 0; i < current->size; i++) {
        if (strcmp(current->arrayTemplates[i]->title, searching) == 0) {
            return current->arrayTemplates[i];
        }
    }
    return NULL;
}

/* Insert a new Template with the given title in Templates */
Template* insertTemplates(Templates* current, const char* title) {
    int i;
    Template* template;

    /* Check if a template with the given title already exists */
    for (i = 0; i < current->size; i++) {
        if (strcmp(current->arrayTemplates[i]->title, title) == 0) {
            return NULL; /* Template already exists, return NULL */
        }
    }
    /* Reallocate memory for arrayTemplates */
    current->size++; /* Increment size */
    current->arrayTemplates = realloc(
        current->arrayTemplates,
        sizeof(*current->arrayTemplates) * current->size
    );
    template = createTemplate(title); /* Create a new template with the given title */
    current->arrayTemplates[current->size - 1] = template; /* Add the new template to arrayTemplates */
    return template; /* Return the newly recorded template */
}
