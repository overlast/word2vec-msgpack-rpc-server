//  Copyright (C) 2014 Toshinori SATO <overlasting _attt_ gmail _dottt_ com>
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  You may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <jansson.h>

// Usage:
// g++ -g -Wall -O4  ./word2vec-msgpack-rpc-server.cc -o /tmp/sample-msgpack-rpc-server -lmsgpack-rpc -lmpio -lmsgpack -ljansson
// /tmp/sample-msgpack-rpc-server -m your_word2vec.bin

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

  model->max_size = 2000; // max length of strings
  model->N = 40; // number of closest words that will be shown
  model->max_w = 50; // max length of vocabulary entries
  model->bestd = (float *)malloc(model->N * sizeof(float));
  model->bestw = (char **)malloc(model->N * sizeof(char *));
  model->st1 = (char *)malloc(model->max_size * sizeof(char));
  model->st_size = 100;
  model->st = (char **)malloc(model->st_size * sizeof(char *));
  model->cn = 0;
  for (i = 0; i < model->st_size; i++) {
    model->st[i] = (char *)malloc(model->max_size * sizeof(char));
  }
  for (i = 0; i < model->N; i++) model->bestw[i] = (char *)malloc(model->max_size * sizeof(char));
  model->bi = (long long *)malloc(model->st_size * sizeof(long long));
  model->bi[0] = -1;
  model->vec = (float *)malloc(model->max_size * sizeof(float));

  fp = fopen(file_path, "rb");
  fscanf(fp, "%lld", &(model->words));
  fscanf(fp, "%lld", &(model->size));
  printf("[word2vec] read the model file...\n");
  printf("[word2vec] words in model = %lld, model window size = %lld\n", model->words, model->size);

  model->vocab = (char *)malloc((long long)(model->words) * model->max_w * sizeof(char));
  model->M = (float *)malloc((long long)(model->words) * (long long)(model->size) * sizeof(float));
  if (model->M == NULL) {
    printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long)(model->words) * model->size * sizeof(float) / 1048576, model->words, model->size);
    return(0);
  }

  for (j = 0; j < model->words; j++) {

    k = 0;
    while (1) {
      (model->vocab)[j * model->max_w + k] = fgetc(fp);
      if (feof(fp) || ((model->vocab)[j * model->max_w + k] == ' ')) break;
      if ((k < model->max_w) && ((model->vocab)[j * model->max_w + k] != '\n')) k++;
    }
    (model->vocab)[j * model->max_w + k] = 0;

    for (k = 0; k < model->size; k++) fread(&((model->M)[k + j * model->size]), sizeof(float), 1, fp);
    len = 0;
    for (k = 0; k < model->size; k++) len += (model->M)[k + j * model->size] * (model->M)[k + j * model->size];
    len = sqrt(len);
    for (k = 0; k < model->size; k++) (model->M)[k + j * model->size] /= len;
  }
  fclose(fp);
  printf("[word2vec] finish to read the model file\n");
  printf("[word2vec] stand by...\n");
  return model;
}

void destroy_word2vec_model(word2vec_model_t *model) {
  long long i = 0;
  free(model->vec);
  free(model->bi);
  for (i = 0; i < model->N; i++) free(model->bestw[i]);
  for (i = 0; i < model->st_size; i++) {
    free(model->st[i]);
  }
  free(model->bestd);
  free(model->bestw);
  free(model->st1);
  free(model->st);
  free(model);
  return;
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

void set_keyword (word2vec_model_t *model, char **keywords, int keyword_num) {
  long long i = 0;
  for (int j = 0; j < keyword_num; j++) {
    int k = 0;
    while (1) {
      if ((keyword_num > 1) && (keywords[j][k] == ' ')) {
        //model->st1[i] = '_';
        //i++;
        k++;
        continue;
      }
      model->st1[i] = keywords[j][k];
      if ((model->st1[i] == '\0') || (i >= (model->max_size - 1))) {
        if (j < (keyword_num - 1)) { // more words
          model->st1[i] = ' ';
          i++;
        } else { // i >= (model->max_size - 1)
          model->st1[i] = '\0';
        }
        break;
      }
      k++;
      i++;
    }
    if (i >= (model->max_size - 1)) {
      break;
    }
  }
  return;
}

void set_keyword_vector (word2vec_model_t *model, char **keywords, int keyword_num) {
  set_keyword(model, keywords, keyword_num);
  make_keyword_vector(model);
  return;
}

void init_word2vec_model (word2vec_model_t *model, char **keywords, int keyword_num) {

  long long i = 0;
  for (i = 0; i < model->N; i++) model->bestd[i] = 0;
  for (i = 0; i < model->N; i++) model->bestw[i][0] = 0;
  for (i = 0; i < model->max_size; i++) model->st1[i] = '\0';
  set_keyword_vector(model, keywords, keyword_num);
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
    if ((model->bi)[b] == -1) continue;
    for (a = 0; a < model->size; a++) (model->vec)[a] += (model->M)[a + (model->bi)[b] * model->size];
  }
  return;
}

void make_analogy_feature_vector(word2vec_model_t* model) {
  long long a;
  for (a = 0; a < model->size; a++) {
    (model->vec)[a] = (model->M)[a + (model->bi)[1] * model->size] - (model->M)[a + (model->bi)[0] * model->size] + (model->M)[a + (model->bi)[2] * model->size];
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
        model->bestd[d] = (model->bestd)[d - 1];
        strcpy(model->bestw[d], (model->bestw)[d - 1]);
      }
      model->bestd[a] = dist;
      strcpy(model->bestw[a], &(model->vocab)[word_id * model->max_w]);
      break;
    }
  }
  return;
}

/*
char* build_json(word2vec_model_t* model) {
  long long i;
  char *result;
  json_t *hash, *set,  *val, *arr1;
  //char *tmp;
  arr1 = json_array();
  if (model->bi[0] != -1) {
    for (i = 0; i < model->N; i++) {
      set = json_object();
      val = json_real((model->bestd)[i]);
      json_object_set(set, (model->bestw)[i], val);
      json_decref(val);
      json_array_append_new(arr1, set);
    }
    hash = json_pack("{so}", "result", arr1);
    result = json_dumps(hash, JSON_INDENT(0));
  } else {
    hash = json_pack("{so}", "result", arr1);
    result = json_dumps(hash, JSON_INDENT(0));
  }
  json_decref(arr1);
  json_decref(hash);
  return result;
}
*/

char* build_json(word2vec_model_t* model, char* method_name, char* keyword) {
  long long i;
  char *result;
  json_t *hash, *val, *item, *arr1;
  //char *tmp;
  arr1 = json_array();
  if (model->bi[0] != -1) {
    for (i = 0; i < model->N; i++) {
      val = json_real((model->bestd)[i]);
      item = json_pack("{ssso}", "term", (model->bestw)[i], "score", val);
      json_array_append_new(arr1, item);
    }
    hash = json_pack("{sssssssosissss}", "format", "json", "method", method_name, "query", keyword, "items", arr1, "total_count", (int)model->N, "status", "OK", "sort", "cosine similarity" );
    result = json_dumps(hash, JSON_INDENT(0));
  } else {
    hash = json_pack("{sssssssosissss}", "format", "json", "method", method_name, "query", keyword, "items", arr1, "total_count", (int)model->N, "status", "NG", "message", "Error: Can't get result using this index term.");
    result = json_dumps(hash, JSON_INDENT(0));
  }
  json_decref(arr1);
  json_decref(hash);
  return result;
}

char* get_null_result(char* keyword, char* status, char* message) {
  int res_size = 0;
  char* res = NULL;
  res_size += strlen((char *)"{\"format\": \"json\", \"query\": \"keyword\", \"items\": [], \"total_count\": 0, \"status\": \"status\", \"message\": \"message\"}");
  res_size += strlen(keyword);
  res_size += strlen(status);
  res_size += strlen(message);
  res = (char*)calloc(res_size + 1, sizeof(char));
  sprintf(res, "{\"format\": \"json\", \"query\": \"%s\", \"items\": [], \"total_count\": 0, \"status\": \"%s\", \"message\": \"%s\"}", keyword, status, message);
  return res;
}

char* distance(char *file_path, char *keyword) {
  char *result;
  float dist;
  long long i;
  word2vec_model_t *model = get_word2vec_model(file_path);

  int keyword_num = 1;
  char** keywords = (char**)malloc(sizeof(char*) * keyword_num);
  char* method_name;

  keywords[0] = keyword;

  init_word2vec_model(model, keywords, keyword_num);
  free(keywords);

  {
    if (!strcmp(model->st1, "")) {
      destroy_word2vec_model(model);
      return get_null_result(keyword, (char*)"OK", (char*)"Length of this query is 0.");
    }
    printf("[distance : %s] get keyword\n", keyword);
    if (model->cn < 1) {
      destroy_word2vec_model(model);
      return get_null_result(keyword, (char*)"NG", (char*)"Error: Can't construct a query vector");
    }
    printf("[distance : %s] search\n", keyword);
    i = search_keywords_on_lexicon(model);
    if (i == -1)  {
      destroy_word2vec_model(model);
      return get_null_result(keyword, (char*)"OK", (char*)"This query isn't an index term.");
    }
    make_feature_vector(model);
    normalize_feature_vector(model);
    printf("[distance : %s] reranking\n", keyword);
    for (i = 0; i < model->words; i++) {
      if (does_match_keywords(model, i) == 1) continue;
      dist = get_cosine_distance(model, i);
      insertion_sort(model, i, dist);
    }
    printf("[distance : %s] generate JSON\n", keyword);
    if (model->bi[0] == -1)  {
      destroy_word2vec_model(model);
      return get_null_result(keyword, (char*)"NG", (char*)"Error: Can't get result using this index term.");
    }

    method_name = (char*)malloc(sizeof(char) * strlen("distance") + 1);
    snprintf(method_name, strlen("distance") + 1, "%s", (char *)"distance");

    result = build_json(model, method_name, keyword);
  }

  free(method_name);
  destroy_word2vec_model(model);

  return result;
}

char* distance(word2vec_model_t *model, char *keyword) {
  char *result;
  float dist;
  long long i;
  char* method_name;

  int keyword_num = 1;
  char** keywords = (char**)malloc(sizeof(char*) * keyword_num);
  keywords[0] = keyword;

  init_word2vec_model(model, keywords, keyword_num);
  free(keywords);

  if (!strcmp(model->st1, "")) return(get_null_result(keyword, (char*)"OK", (char*)"Length of this query is 0."));
  printf("[distance : %s] get keyword\n", keyword);
  if (model->cn < 1) return(get_null_result(keyword, (char*)"NG", (char*)"Error: Can't construct a query vector"));
  printf("[distance : %s] search\n", keyword);
  i = search_keywords_on_lexicon(model);
  if (i == -1) return(get_null_result(keyword, (char*)"OK", (char*)"This query isn't an index term."));
  make_feature_vector(model);
  normalize_feature_vector(model);
  printf("[distance : %s] reranking\n", keyword);
  for (i = 0; i < model->words; i++) {
    if (does_match_keywords(model, i) == 1) continue;
    dist = get_cosine_distance(model, i);
    insertion_sort(model, i, dist);
  }
  printf("[distance : %s] generate JSON\n", keyword);
  if (model->bi[0] == -1) return(get_null_result(keyword, (char*)"NG", (char*)"Error: Can't get result using this index term."));

  method_name = (char*)malloc(sizeof(char) * strlen("distance") + 1);
  snprintf(method_name, strlen("distance") + 1, "%s", (char *)"distance");

  result = build_json(model, method_name, keyword);

  free(method_name);

  return result;
}

char* analogy(char *file_path, char *keyword0, char *keyword1, char *keyword2) {
  char *result;
  float dist;
  long long i;
  word2vec_model_t *model = get_word2vec_model(file_path);

  int query_str_size = strlen(keyword0) + strlen(keyword1) + strlen(keyword2) + 3;
  char* query_str = (char*)malloc(sizeof(char) * query_str_size);
  char* method_name;
  int keyword_num = 3;
  char** keywords = (char**)malloc(sizeof(char*) * keyword_num);

  snprintf(query_str, query_str_size, "%s %s %s", keyword0, keyword1, keyword2);

  keywords[0] = keyword0;
  keywords[1] = keyword1;
  keywords[2] = keyword2;

  init_word2vec_model(model, keywords, keyword_num);
  free(keywords);

  {
    if (!strcmp(model->st1, "")) {
      destroy_word2vec_model(model);
      result = get_null_result(query_str, (char*)"OK", (char*)"Length of this query is 0.");
      free(query_str);
      return result;
    }
    printf("[analogy : %s] get keyword\n", query_str);
    if (model->cn < 3) {
      destroy_word2vec_model(model);
      result = get_null_result(query_str, (char*)"NG", (char*)"Error: Can't construct a query vector");
      free(query_str);
      return result;
    }
    printf("[analogy : %s] search\n", query_str);
    i = search_keywords_on_lexicon(model);
    if (i == -1)  {
      destroy_word2vec_model(model);
      result = get_null_result(query_str, (char*)"OK", (char*)"This query isn't an index term.");
      free(query_str);
      return result;
    }
    make_analogy_feature_vector(model);
    normalize_feature_vector(model);
    printf("[analogy : %s] reranking\n", query_str);
    for (i = 0; i < model->words; i++) {
      if (i == (model->bi)[0]) continue;
      if (i == (model->bi)[1]) continue;
      if (i == (model->bi)[2]) continue;
      if (does_match_keywords(model, i) == 1) continue;
      dist = get_cosine_distance(model, i);
      insertion_sort(model, i, dist);
    }
    printf("[analogy : %s] generate JSON\n", query_str);
    if (model->bi[0] == -1)  {
      destroy_word2vec_model(model);
      result = get_null_result(query_str, (char*)"NG", (char*)"Error: Can't get result using this index term.");
      free(query_str);
      return result;
    }

    method_name = (char*)malloc(sizeof(char) * strlen("analogy") + 1);
    snprintf(method_name, strlen("analogy") + 1, "%s", (char *)"analogy");

    result = build_json(model, method_name, query_str);
  }

  free(query_str);
  free(method_name);
  destroy_word2vec_model(model);

  return result;
}

char* analogy(word2vec_model_t *model, char *keyword0, char *keyword1, char *keyword2) {
  char *result;
  float dist;
  long long i;

  int query_str_size = strlen(keyword0) + strlen(keyword1) + strlen(keyword2) + 3;
  char* query_str = (char*)malloc(sizeof(char) * query_str_size);
  char* method_name;
  int keyword_num = 3;
  char** keywords = (char**)malloc(sizeof(char*) * keyword_num);

  snprintf(query_str, query_str_size, "%s %s %s", keyword0, keyword1, keyword2);

  keywords[0] = keyword0;
  keywords[1] = keyword1;
  keywords[2] = keyword2;

  init_word2vec_model(model, keywords, keyword_num);
  free(keywords);

  if (!strcmp(model->st1, "")) {
    result = get_null_result(query_str, (char*)"OK", (char*)"Length of this query is 0.");
    free(query_str);
    return result;
  }

  printf("[analogy : %s] get keyword\n", query_str);

  if (model->cn < 3) {
    result = get_null_result(query_str, (char*)"NG", (char*)"Error: Can't construct a query vector");
    free(query_str);
    return result;
  }

  printf("[analogy : %s] search\n", query_str);
  i = search_keywords_on_lexicon(model);

  if (i == -1) {
    result = get_null_result(query_str, (char*)"OK", (char*)"This query isn't an index term.");
    free(query_str);
    return result;
  }
  make_analogy_feature_vector(model);
  normalize_feature_vector(model);
  printf("[analogy : %s] reranking\n", query_str);

  for (i = 0; i < model->words; i++) {
    if (i == (model->bi)[0]) continue;
    if (i == (model->bi)[1]) continue;
    if (i == (model->bi)[2]) continue;
    if (does_match_keywords(model, i) == 1) continue;
    dist = get_cosine_distance(model, i);
    insertion_sort(model, i, dist);
  }

  printf("[analogy : %s] generate JSON\n", query_str);

  if (model->bi[0] == -1) {
    result = get_null_result(query_str, (char*)"NG", (char*)"Error: Can't get result using this index term.");
    free(query_str);
    return result;
  }

  method_name = (char*)malloc(sizeof(char) * strlen("analogy") + 1);
  snprintf(method_name, strlen("analogy") + 1, "%s", (char *)"analogy");

  result = build_json(model, method_name, query_str);

  free(query_str);
  free(method_name);

  return result;
}
