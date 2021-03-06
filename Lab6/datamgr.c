#include <stdio.h>
#include "datamgr.h"
#include <sys/time.h>
#include <stdlib.h>

FILE *sensor_map;
FILE *sensor_data;

struct list{
  int size;
  list_node_ptr_t first_node;
  /*void * free;
  void * copy;
  void * compare;
  void * print;*/
};

struct list_node{
  element_ptr_t data;
  list_node_ptr_t next;
  list_node_ptr_t prev;
};

struct running_avg{
  int count;
  float temp[5];
  float running_avg;
};

struct element{
  unsigned int sensor_id;
  unsigned int room_id;
  running_avg_ptr_t avg_data;
  time_t last_modified;
  //mogen er nog meer bij
};



int list_errno = LIST_NO_ERROR;

list_ptr_t list_create 	( // callback functions (ptr_to_callback_func)(arguments)
        /*void (*element_copy)(element_ptr_t *dest_element, element_ptr_t src_element),
        void (*element_free)(element_ptr_t *element),
        int (*element_compare)(element_ptr_t x, element_ptr_t y),
        void (*element_print)(element_ptr_t element)*/)
        {

  list_ptr_t list = malloc(sizeof(list_t));    // returned een pointer = list_ptr
  if(list == NULL){
    list_errno = LIST_MEMORY_ERROR;
    return NULL;
  }

  list->size = 0;
  list->first_node = NULL;
  //list->free = element_free;
  //list->copy = element_copy;
  //list->compare = element_compare;
  //list->print = element_print;

  return list;
}

void list_free(list_ptr_t* list){
  int i;
  for(i = 0; i <= (*list)->size-1; i++){
    list_free_at_index(*list, i);
  }

  if(list_errno == LIST_EMPTY_ERROR){
    //List was empty, so list_free_at_index() has set errno to LIST_EMPTY_ERROR;
    //Bypass the error.
    list_errno = LIST_NO_ERROR;
  }
  free(*list);
  *list = NULL;
}

int list_size( list_ptr_t list ){
  return list->size;
}

list_ptr_t list_insert_at_index( list_ptr_t list, element_ptr_t element, int index){

  list_node_ptr_t new_node = malloc(sizeof(list_node_t));

  new_node->data = element;

  if(list == NULL){
    list_errno = LIST_INVALID_ERROR;
    return NULL;
  }

  if(list->size <= 0){
    new_node->next = NULL;
    new_node->prev = NULL;
    list->first_node = new_node;
    list->size++;
    return list;
  }

  else if(index >= list->size){
    list_node_ptr_t current_node = list_get_reference_at_index(list, index);

    current_node->next = new_node;
    new_node->next = NULL;
    new_node->prev = current_node;
    list->size++;
    return list;
  }

  else{

    list_node_ptr_t current_node = list_get_reference_at_index(list, index);
    if(index <= 0){
      new_node->next = list->first_node;
      list->first_node = new_node;
      new_node->prev = NULL;
      new_node->next->prev = new_node;
    }

    else{
      new_node->next = current_node;
      new_node->prev = current_node->prev;
      current_node->prev->next = new_node;
      current_node->prev = new_node;
    }
    list->size++;
    return list;
  }
}

list_ptr_t list_remove_at_index( list_ptr_t list, int index){
  if(list == NULL){
    list_errno = LIST_INVALID_ERROR;
    return NULL;
  }

  if(list->size == 0){
    list_errno = LIST_EMPTY_ERROR;
    return list;
  }

  if(index <= 0){
    //remove first_node
    list->first_node->next->prev = NULL;
    list->first_node = list->first_node->next;
    list->size--;

  }

  else if(index >= (list->size - 1)){
      list_node_ptr_t last_node = list_get_reference_at_index(list, index);
      if(last_node->prev == NULL){
        list->first_node = NULL;
      }
      else{
        last_node->prev->next = NULL;
      }
      last_node->prev = NULL;
      list->size--;
    }

  else{
    list_node_ptr_t current_node = list_get_reference_at_index(list, index);
    current_node->next->prev = current_node->prev;
    current_node->prev->next = current_node->next;
    list->size--;
  }
  return list;
}

list_ptr_t list_free_at_index( list_ptr_t list, int index){
  if(list == NULL){
    list_errno = LIST_INVALID_ERROR;
    return NULL;
  }

  if(list->size == 0){
    list_errno = LIST_EMPTY_ERROR;
    return list;
  }

  list_node_ptr_t current_node = list_get_reference_at_index(list, index);
  list = list_remove_at_index(list, index);
  free(current_node);
  return list;
}

list_node_ptr_t list_get_reference_at_index( list_ptr_t list, int index ){

  if(list == NULL){
    list_errno = LIST_INVALID_ERROR;
    return NULL;
  }

  if(index <= 0){
    return list->first_node;
  }

  else if(index >= (list->size - 1)){
    list_node_ptr_t current_node = list->first_node;
    while(current_node->next != NULL){
      current_node = current_node->next;
    }
    return current_node;
  }

  else{
    list_node_ptr_t current_node = list->first_node;
    int i = 0;
    while(i < index){
      current_node = current_node->next;
      i++;
    }
    return current_node;
  }
  return NULL;
}

element_ptr_t list_get_element_at_index( list_ptr_t list, int index ){
  list_node_ptr_t temp = list_get_reference_at_index(list, index);
  if(list == NULL){
    return NULL;
  }
  return temp->data;
}

int list_get_index_of_element(list_ptr_t list, element_ptr_t element ){

  if(list == NULL){
    list_errno = LIST_INVALID_ERROR;
    return -1;
  }

  if(list->first_node == NULL){
    list_errno = LIST_EMPTY_ERROR;
    return -1;
  }

  int i;
  for(i = 0; i <= list->size-1; i++){
    if(list_get_element_at_index(list, i) == element){
      return i;
    }
  }
  return -1;
}

void list_print(list_ptr_t list){
  list_node_ptr_t current_node = list->first_node;
  while(current_node != NULL){
    printf("room_id: %d en sensor_id: %d\n", current_node->data->room_id, current_node->data->sensor_id);

    current_node = current_node->next;
    }
}

void read_room_map(list_ptr_t list){
  int i, room_id, sensor_id;
  i = 0;

  sensor_map = fopen(MAP_FILE,"r");
  while(fscanf(sensor_map, "%d %d ", &room_id, &sensor_id) == 2){
    element_ptr_t name = malloc(sizeof(element_t));
    name->room_id = room_id;
    name->sensor_id = sensor_id;
    name->last_modified = 0;
    running_avg_ptr_t temperature_data = malloc(sizeof(running_avg_t));
    name->avg_data = temperature_data;
    temperature_data->count = 0;
    list_insert_at_index(list, name, i);
    i++;
  }
}

void print_sensor_data(){
  int i, size;
  short sensor_id;
  double temp;
  time_t last_modified;

  sensor_data = fopen(SENSOR_FILE, "r");
  //get file size
  //http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
  fseek(sensor_data, 0L, SEEK_END);
  size = ftell(sensor_data);
  //lines = number of total inputs
  int lines = size/(sizeof(short) + sizeof(double) + sizeof(time_t));
  //printf("%d\n", lines);
  //Terug naar begin van bestand voor fread's
  fseek(sensor_data, 0L, SEEK_SET);
  for(i = 0; i<lines; i++){

    fread(&sensor_id, sizeof(short), 1, sensor_data);
    fread(&temp, sizeof(double), 1, sensor_data);
    fread(&last_modified, sizeof(time_t), 1, sensor_data);
    printf("%d\t%f\t%ld\n", sensor_id, temp, last_modified);
  }
  fclose(sensor_data);
}

void parse_sensor_data(list_ptr_t list){
  int i, size;
  short sensor_id;
  double temp;
  time_t last_modified;

  sensor_data = fopen(SENSOR_FILE, "r");
  //get file size
  //http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
  fseek(sensor_data, 0L, SEEK_END);
  size = ftell(sensor_data);
  //lines = number of total inputs
  int lines = size/(sizeof(short) + sizeof(double) + sizeof(time_t));
  //printf("%d\n", lines);
  //Terug naar begin van bestand voor fread's
  fseek(sensor_data, 0L, SEEK_SET);
  for(i = 0; i<lines; i++){

    fread(&sensor_id, sizeof(short), 1, sensor_data);
    fread(&temp, sizeof(double), 1, sensor_data);
    fread(&last_modified, sizeof(time_t), 1, sensor_data);

    //printf("%f\n", temp);
    if(update_sensor_node(list, sensor_id, temp, last_modified) != 0){
      printf("Failed to update running_avg");
    }
  }
  fclose(sensor_data);
}

int update_sensor_node(list_ptr_t list, unsigned int sensor_id, float temp, time_t last_modified){
  if(list == NULL){
    //list ptr NULL
    list_errno = LIST_INVALID_ERROR;
    return -1;
  }

  float total;
  int i;
  list_node_ptr_t current_node = list->first_node;

  //search correct sensor_node
  while(current_node->data->sensor_id != sensor_id){
    current_node = current_node->next;
  }
  //algorithm for determining the running_avg

  if(current_node->data->avg_data->count < 5){
    current_node->data->avg_data->temp[current_node->data->avg_data->count] = temp;
    current_node->data->avg_data->count++;
  }
  //array containing last 5 temps is saturated, shift every value 1 place and add latest.
  else{
    for(i = 0; i < 5; i++){
      current_node->data->avg_data->temp[i] = current_node->data->avg_data->temp[i+1];
    }
  }

  total = 0;
  for(i = 0; i<current_node->data->avg_data->count; i++){
    total = total + current_node->data->avg_data->temp[i];
  }

  current_node->data->avg_data->running_avg = total/current_node->data->avg_data->count;

  if(current_node->data->avg_data->running_avg < SET_MIN_TEMP){
    fprintf(stderr, "Room %d is too cold\n", current_node->data->room_id);
  }

  if(current_node->data->avg_data->running_avg > SET_MAX_TEMP){
    fprintf(stderr, "Room %d is too hot\n", current_node->data->room_id);
  }

  //update last_modified

  if((int)last_modified > (int)current_node->data->last_modified){
    current_node->data->last_modified = last_modified;
  }

  printf("%f\n",(float)current_node->data->last_modified);

  return 0;
}
