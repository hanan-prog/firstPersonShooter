#ifndef MODELS_H
#define MODELS_H

#include "glad/glad.h"
#include "glm/glm.hpp"

#include <cstdio>
#include <fstream>

using namespace std;


// smart person would've used std::vector but oh well

typedef struct model_t {
  const char *name;
  int start;
  int num_vertices;
  float *data;
  model_t *next_model;
} model_t;

// linked list of models
typedef struct model_list_t {
  int len;
  int total_vertices;
  model_t *root;
} model_list_t;

class Models {
public:
  Models() { models_ = init_model_list(); }
  ~Models() { clear_models(models_); }

  void load_model(const char *fname) {
    add_model(fname, models_);
  }

  void load_model(const char *fname, int size) {
    add_model(fname, models_, size);
  }

  float *get_all_model_data() { return combine_models(models_); }
  float *combined_model_data(model_t *model1, model_t *model2, int size) {
    int total_vertices = model1->num_vertices + model2->num_vertices;
    int modelDataSize  = total_vertices * size; 

    float *modelData = new float[modelDataSize];

    // copy model1 data
    int model1_size = model1->num_vertices * size;
    std::copy(model1->data, model1->data + model1_size, modelData);

    // copy model2 data
    int model2_size = model2->num_vertices * size;
    std::copy(model2->data, model2->data + model2_size, modelData + model1_size);

    return modelData;
  }
  int get_total_vertices() { return models_->total_vertices; }

  model_list_t *get_models() { return models_; }





private:
  model_list_t *models_;
  model_list_t *init_model_list() {
    model_list_t *res = (model_list_t *)malloc(sizeof(model_list_t));
    res->len = 0;
    res->total_vertices = 0;
    res->root = nullptr;

    return res;
  }

  void clear_models(model_list_t *model_list) {
    model_t *current = model_list->root;
    while (current != nullptr) {
      model_t *next = current->next_model;
      delete[] current->data;
      free(current);
      current = next;
    }
    model_list->root = nullptr;
    model_list->len = 0;
    model_list->total_vertices = 0;
    free(model_list);
  }

  int add_model(const char *fname, model_list_t *model_list, int size = 8) {
    if (model_list == nullptr) {
      printf("model list is null");
      return 1;
    }

    ifstream modelFile;
    modelFile.open(fname);
    if (!modelFile.is_open()) {
      // problem opening
      printf("can't open file\n");
      return 1;
    }

    model_t *new_model = (model_t *)malloc(sizeof(model_t));
    if (new_model == NULL) {
      printf("can't allocate memory ");
      modelFile.close();
      return 1;
    }

    int num_lines = 0;
    modelFile >> num_lines;
    new_model->data = new float[num_lines];
    for (int i = 0; i < num_lines; i++) {
      modelFile >> new_model->data[i];
    }

    new_model->name = fname;

    new_model->num_vertices = num_lines / size;
    new_model->start = model_list->total_vertices;
    new_model->next_model = nullptr;
    model_list->total_vertices += new_model->num_vertices;
    model_list->len++;

    if (model_list->root == nullptr) {
      // empty list new model will be head of that list
      model_list->root = new_model;
    } else {
      // search for the next empty place in the list and have new model point to
      // it
      model_t *prev = model_list->root;
      while (prev->next_model != nullptr) {
        prev = prev->next_model;
      }
      prev->next_model = new_model;
      new_model->next_model = nullptr;
    }

    modelFile.close();
    return 0;
  }

  // models with 8 floats per vertex
 

  // combines all model data into one buffer and returns that buffer
  float *combine_models(model_list_t *models, int size = 8) {

    // only combinemodels with same size
  
    int modelDataSize = models->total_vertices * size;
    float *modelData = new float[modelDataSize];

    model_t *curr_model = models->root;
    int offset = 0;
    while (curr_model != nullptr) {

      // copy over data
      float *curr_size = curr_model->data + curr_model->num_vertices * size;
      copy(curr_model->data, curr_size, modelData + offset);
      offset += curr_model->num_vertices * size;
      curr_model = curr_model->next_model;
    }
    return modelData;
  }
};

#endif // MODELS_H