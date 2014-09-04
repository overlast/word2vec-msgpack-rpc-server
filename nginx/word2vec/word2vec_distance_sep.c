//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <jansson.h>

typedef struct {
  long long max_size;
  long long N;
  long long max_w;
  long long words;
  long long size;
  long long cn;
  long long st_size;
  long long *bi;
  char *vocab;
  float *M;
  float *bestd;
  char **bestw;
  char **st;
  char *st1;
  float *vec;
} word2vec_model_t;

static word2vec_model_t* get_word2vec_model(char *file_path) {
  word2vec_model_t *model = (word2vec_model_t *)malloc(sizeof(word2vec_model_t));
  long long i, j, k;
  float len;
  FILE *fp;
  char ch;

  model->max_size = 2000; // max length of strings
  model->N = 40; // number of closest words that will be shown
  model->max_w = 50; // max length of vocabulary entries
  model->bestd = (float *)malloc(model->N * sizeof(float));
  model->bestw = (char **)malloc(model->N * sizeof(char *));
  model->st1 = (char *)malloc(model->max_size * sizeof(char));
  model->st_size = 100;
  model->st = (char **)malloc(model->st_size * sizeof(char *));
  for (i = 0; i < model->st_size; i++) {
    model->st[i] = (char *)malloc(model->max_size * sizeof(char));
  }
  for (i = 0; i < model->N; i++) model->bestw[i] = (char *)malloc(model->max_size * sizeof(char));
  model->bi = (long long *)malloc(model->st_size * sizeof(long long));
  model->vec = (float *)malloc(model->st_size * sizeof(float));

  fp = fopen(file_path, "rb");
  fscanf(fp, "%lld", model->words);
  fscanf(fp, "%lld", model->size);
  model->vocab = (char *)malloc((long long)(model->words) * model->max_w * sizeof(char));
  model->M = (float *)malloc((long long)(model->words) * (long long)(model->size) * sizeof(float));
  if (model->M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)(model->words) * model->size * sizeof(float) / 1048576, model->words, model->size);
    return(0);
  }

  for (j = 0; j < model->words; j++) {
    fscanf(fp, "%s%c", &((model->vocab)[j * model->max_w]), &ch);
    for (k = 0; k < model->size; k++) fread(&((model->M)[k + j * model->size]), sizeof(float), 1, fp);
    len = 0;
    for (k = 0; k < model->size; k++) len += (model->M)[k + j * model->size] * (model->M)[k + j * model->size];
    len = sqrt(len);
    for (k = 0; k < model->size; k++) (model->M)[k + j * model->size] /= len;
  }
  fclose(fp);
  return model;
}

int fexist(const char *file_path) {
  FILE  *fp;
  if ((fp = fopen(file_path, "r")) == NULL) {
    return (0);
  }
  fclose(fp);
  return (1);
}

void make_keyword_vector (word2vec_model_t *model) {
  model->cn = 0;
  long long i = 0;
  long long j = 0;
  while (1) {
    model->st[model->cn][i] = model->st1[j];
    i++;
    j++;
    model->st[model->cn][i] = 0;
    if (model->st1[j] == 0) break;
    if (model->st1[j] == ' ') {
      model->cn++;
      i = 0;
      j++;
    }
  }
  model->cn++;
  return;
}


void set_keyword (word2vec_model_t *model, char *keyword) {
  long long i = 0;
  while (1) {
    model->st1[i] = keyword[i];
    if ((model->st1[i] == '\0') || (i >= (model->max_size - 1))) {
      model->st1[i] = '\0';
      break;
    }
    i++;
  }
  return;
}

void set_keyword_vector (word2vec_model_t *model, char *keyword) {
  set_keyword(model, keyword);
  make_keyword_vector(model);
  return;

}

void init_word2vec_model (word2vec_model_t *model, char *keyword) {
  long long i = 0;
  for (i = 0; i < model->N; i++) model->bestd[i] = 0;
  for (i = 0; i < model->N; i++) model->bestw[i][0] = 0;
  for (i = 0; i < model->max_size; i++) model->st1[i] = '\0';
  set_keyword_vector(model, keyword);
  for (i = 0; i < model->size; i++) model->vec[i] = 0;
  return;
}

long long search_keywords_on_lexicon(word2vec_model_t *model) {
  long long i = 0;
  long long b = 0;
  for (i = 0; i < model->cn; i++) {
    for (b = 0; b < model->words; b++) if (!strcmp(&(model->vocab[b * model->max_w]), model->st[i])) break;
    if (b == model->words) b = -1;
    model->bi[i] = b;
    if (b == -1) break;
  }
  return b;
}

void make_feature_vector(word2vec_model_t* model) {
  long long a;
  long long b;
  for (b = 0; b < model->cn; b++) {
    if (model->bi[b] == -1) continue;
    for (a = 0; a < model->size; a++) model->vec[a] += (model->M)[a + model->bi[b] * model->size];
  }
  return;
}

void normalize_feature_vector(word2vec_model_t* model) {
  long long i;
  float len = 0;
  for (i = 0; i < model->size; i++) len += model->vec[i] * model->vec[i];
  len = sqrt(len);
  for (i = 0; i < model->size; i++) model->vec[i] /= len;
  return;
}

int does_match_keywords(word2vec_model_t* model, long long word_id) {
  int a = 0;
  long long b;
  for (b = 0; b < model->cn; b++) if ((model->bi)[b] == word_id) a = 1;
  return a;
}

float get_cosine_distance(word2vec_model_t* model, long long word_id) {
  float dist = 0;
  long long a;
  for (a = 0; a < model->size; a++) dist += (model->vec)[a] * (model->M)[a + word_id * model->size];
  return dist;
}

void insertion_sort(word2vec_model_t* model, long long word_id, float dist) {
  long long a, d;
  for (a = 0; a < model->N; a++) {
    if (dist > (model->bestd)[a]) {
      for (d = model->N - 1; d > a; d--) {
        model->bestd[d] = model->bestd[d - 1];
        model->bestw[d] = model->bestw[d - 1];
      }
      model->bestd[a] = dist;
      strcpy(model->bestw[a], &(model->vocab[word_id * model->max_w]));
      break;
    }
  }
  return;
}

char* build_json(word2vec_model_t* model) {
  char *json;
  long long i;
  size_t is_JSON_COMPACT = 1;
  json_t *json_root;
  json_t *word_array = json_array();
  json_t *dist_array = json_array();
  for (i = 0; i < model->N; i++) {
    json_array_append_new(word_array, json_string(model->bestw[i]));
    json_array_append_new(dist_array, json_real((double)(model->bestd[i])));
  }
  json_root = json_pack("{soso}", "keyword", word_array , "score", dist_array);
  json = json_dumps(json_root, is_JSON_COMPACT);
  json_decref(json_root);
  json_decref(word_array);
  json_decref(dist_array);
  return json;
}

int main(int argc, char **argv) {
  word2vec_model_t *model;
  char *file_path;
  char *keyword;
  long long i;
  float dist;
  char *result;
  if (argc < 2) {
    printf("you syould set file_path as argvhh\n");
    return 1;
  }
  strcpy(file_path, argv[1]);
  strcpy(keyword, argv[2]);
  if ((file_path == NULL) || (file_path == '\0') || (! fexist(file_path))) {
    printf("You should set file_path which contains word projections in the BINARY FORMAT\n");
    return 1;
  }
  model = get_word2vec_model(file_path);

  {
    init_word2vec_model(model, keyword);
    if (!strcmp(model->st1, "")) return(0);
    if (model->cn < 1) return(0);
    i = search_keywords_on_lexicon(model);
    if (i == -1) return(0);
    make_feature_vector(model);
    normalize_feature_vector(model);
    for (i = 0; i < model->words; i++) {
      if (does_match_keywords(model, i) == 1) continue;
      dist = get_cosine_distance(model, i);
      insertion_sort(model, i, dist);
    }
    result = build_json(model);
    printf("%s\n", result);
  }

  return 0;
}
