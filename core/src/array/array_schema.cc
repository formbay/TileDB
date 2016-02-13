/*
 * @file   array_schema.cc
 * @author Stavros Papadopoulos <stavrosp@csail.mit.edu>
 *
 * @section LICENSE
 *
 * The MIT License
 * 
 * @copyright Copyright (c) 2014 Stavros Papadopoulos <stavrosp@csail.mit.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * @section DESCRIPTION
 *
 * This file implements the ArraySchema class.
 */

#include "array_schema.h"
#include "constants.h"
#include "utils.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>

/* ****************************** */
/*             MACROS             */
/* ****************************** */

#if VERBOSE == 1
#  define PRINT_ERROR(x) std::cerr << "[TileDB] Error: " << x << ".\n" 
#  define PRINT_WARNING(x) std::cerr << "[TileDB] Warning: " \
                                     << x << ".\n"
#elif VERBOSE == 2
#  define PRINT_ERROR(x) std::cerr << "[TileDB::ArraySchema] Error: " \
                                   << x << ".\n" 
#  define PRINT_WARNING(x) std::cerr << "[TileDB::ArraySchema] Warning: " \
                                     << x << ".\n"
#else
#  define PRINT_ERROR(x) do { } while(0) 
#  define PRINT_WARNING(x) do { } while(0) 
#endif

/* ****************************** */
/*   CONSTRUCTORS & DESTRUCTORS   */
/* ****************************** */

ArraySchema::ArraySchema() {
  cell_num_per_tile_ = -1;
  coords_for_hilbert_ = NULL;
  domain_ = NULL;
  hilbert_curve_ = NULL;
  tile_extents_ = NULL;
  tile_domain_ = NULL;
}

ArraySchema::~ArraySchema() {
  if(domain_ != NULL)
    free(domain_);

  if(hilbert_curve_ != NULL)
    delete hilbert_curve_;

  if(tile_extents_ != NULL)
    free(tile_extents_);

  if(tile_domain_ != NULL)
    free(tile_domain_);

  if(coords_for_hilbert_ != NULL)
    delete [] coords_for_hilbert_;
}

/* ****************************** */
/*            ACCESSORS           */
/* ****************************** */

const std::string& ArraySchema::array_name() const {
  return array_name_;
}

const std::string& ArraySchema::attribute(int attribute_id) const {
  assert(attribute_id>= 0 && attribute_id <= attribute_num_);

  return attributes_[attribute_id];
}

int ArraySchema::attribute_id(const std::string& attribute) const {
  // Special case - coordinates
  if(attribute == TILEDB_COORDS_NAME)
    return attribute_num_;

  for(int i=0; i<attribute_num_; ++i) {
    if(attributes_[i] == attribute)
      return i;
  }

  // Attribute not found
  return TILEDB_AS_ERR;
}

int ArraySchema::attribute_num() const {
  return attribute_num_;
}

const std::vector<std::string>& ArraySchema::attributes() const {
  return attributes_;
}

int64_t ArraySchema::capacity() const {
  return capacity_;
}

ArraySchema::Compression ArraySchema::compression(int attribute_id) const {
  assert(attribute_id >= 0 && attribute_id <= attribute_num_);

  return compression_[attribute_id];
}

int64_t ArraySchema::cell_num_per_tile() const {
  return cell_num_per_tile_;
}

ArraySchema::CellOrder ArraySchema::cell_order() const {
  return cell_order_;
}

size_t ArraySchema::cell_size(int attribute_id) const {
  return cell_sizes_[attribute_id];
}

size_t ArraySchema::coords_size() const {
  return cell_sizes_[attribute_num_];
}

const std::type_info* ArraySchema::coords_type() const {
  return type(attribute_num_);
}

bool ArraySchema::dense() const {
  return dense_;
}

int ArraySchema::dim_num() const {
  return dim_num_;
}

const void* ArraySchema::domain() const {
  return domain_;
}

int ArraySchema::get_attribute_ids(
    const std::vector<std::string>& attributes,
    std::vector<int>& attribute_ids) const {
  // Initialization
  attribute_ids.clear();
  int attribute_num = attributes.size();
  int id;

  // Get attribute ids
  for(int i=0; i<attribute_num; ++i) {
    id = attribute_id(attributes[i]);
    if(id == TILEDB_AS_ERR) {
      PRINT_ERROR(std::string("Cannot get attribute id; Attribute '") + 
                  attributes[i] + "' does not exist");
      return TILEDB_AS_ERR;
    } else {
      attribute_ids.push_back(id);
    }
  }

  // Success
  return TILEDB_AS_OK;
}

void ArraySchema::print() const {
  // Array name
  std::cout << "Array name:\n\t" << array_name_ << "\n";
  // Dimension names
  std::cout << "Dimension names:\n";
  for(int i=0; i<dim_num_; ++i)
    std::cout << "\t" << dimensions_[i] << "\n";
  // Attribute names
  std::cout << "Attribute names:\n";
  for(int i=0; i<attribute_num_; ++i)
    std::cout << "\t" << attributes_[i] << "\n";
  // Domain
  std::cout << "Domain:\n";
  if(types_[attribute_num_] == &typeid(int)) {
    int* domain_int = (int*) domain_; 
    for(int i=0; i<dim_num_; ++i) {
      std::cout << "\t"<<  dimensions_[i] << ": [" << domain_int[2*i] << ","
                                          << domain_int[2*i+1] << "]\n";
    }
  } else if(types_[attribute_num_] == &typeid(int64_t)) {
    int64_t* domain_int64 = (int64_t*) domain_; 
    for(int i=0; i<dim_num_; ++i) {
      std::cout << "\t" << dimensions_[i] << ": [" << domain_int64[2*i] << ","
                                          << domain_int64[2*i+1] << "]\n";
    }
  } else if(types_[attribute_num_] == &typeid(float)) {
    float* domain_float = (float*) domain_; 
    for(int i=0; i<dim_num_; ++i) {
      std::cout << "\t" << dimensions_[i] << ": [" << domain_float[2*i] << ","
                                          << domain_float[2*i+1] << "]\n";
    }
  } else if(types_[attribute_num_] == &typeid(double)) {
    double* domain_double = (double*) domain_; 
    for(int i=0; i<dim_num_; ++i) {
      std::cout << "\t" << dimensions_[i] <<  ": [" << domain_double[2*i] << ","
                                          << domain_double[2*i+1] << "]\n";
    }
  }
  // Types
  std::cout << "Types:\n";
  for(int i=0; i<attribute_num_; ++i) {
    if(*types_[i] == typeid(char)) {
      std::cout << "\t" << attributes_[i] << ": char[";
    } else if(*types_[i] == typeid(int)) {
      std::cout << "\t" << attributes_[i] << ": int32[";
    } else if(*types_[i] == typeid(int64_t)) {
      std::cout << "\t" << attributes_[i] << ": int64[";
    } else if(*types_[i] == typeid(float)) {
      std::cout << "\t" << attributes_[i] << ": float32[";
    } else if(*types_[i] == typeid(double)) {
      std::cout << "\t" << attributes_[i] << ": float64[";
    }
    if(val_num_[i] == TILEDB_AS_VAR_SIZE)
      std::cout << "var]\n";
    else
      std::cout << val_num_[i] << "]\n";
  }
  if(key_value_)
    std::cout << "\tCoordinates: char: var\n";
  else if(*types_[attribute_num_] == typeid(int))
    std::cout << "\tCoordinates: int32\n";
  else if(*types_[attribute_num_] == typeid(int64_t))
    std::cout << "\tCoordinates: int64\n";
  else if(*types_[attribute_num_] == typeid(float))
    std::cout << "\tCoordinates: float32\n";
  else if(*types_[attribute_num_] == typeid(double))
    std::cout << "\tCoordinates: float64\n";
  // Cell sizes
  std::cout << "Cell sizes (in bytes):\n";
  for(int i=0; i<=attribute_num_; ++i) {
    std::cout << "\t" << ((i==attribute_num_) ? "Coordinates"  
                                              : attributes_[i]) 
                      << ": ";
    if(cell_sizes_[i] == TILEDB_AS_VAR_SIZE)
      std::cout << "var\n"; 
    else
      std::cout << cell_sizes_[i] << "\n"; 
  }
  // Dense
  std::cout << "Dense:\n\t" << (dense_ ? "true" : "false") << "\n";
  // Key-value
  std::cout << "Key-value:\n\t" << (key_value_ ? "true" : "false") << "\n";
  // Tile type
  std::cout << "Tile types:\n\t" 
            << (tile_extents_ == NULL ? "irregular" : "regular") << "\n";
  // Tile order
  std::cout << "Tile order:\n\t";
  if(tile_extents_ == NULL) {
    std::cout << "-\n";
  } else {
    if(tile_order_ == TILEDB_AS_TO_COLUMN_MAJOR)
      std::cout << "column-major\n";
    else if(tile_order_ == TILEDB_AS_TO_HILBERT)
      std::cout << "hilbert\n";
    else if(tile_order_ == TILEDB_AS_TO_ROW_MAJOR)
      std::cout << "row-major\n";
  }
  // Cell order
  std::cout << "Cell order:\n\t";
  if(cell_order_ == TILEDB_AS_CO_COLUMN_MAJOR)
    std::cout << "column-major\n";
  else if(cell_order_ == TILEDB_AS_CO_HILBERT)
    std::cout << "hilbert\n";
  else if(cell_order_ == TILEDB_AS_CO_ROW_MAJOR)
    std::cout << "row-major\n";
  // Capacity
  std::cout << "Capacity:\n\t";
  if(tile_extents_ != NULL)
    std::cout << "-\n";
  else
    std::cout << capacity_ << "\n";
  // Tile extents
  std::cout << "Tile extents:\n";
  if(tile_extents_ == NULL) {
    std::cout << "-\n"; 
  } else {
    if(types_[attribute_num_] == &typeid(int)) {
      int* tile_extents_int = (int*) tile_extents_;
      for(int i=0; i<dim_num_; ++i)
        std::cout << "\t" << dimensions_[i] << ": " << tile_extents_int[i] << "\n";
    } else if(types_[attribute_num_] == &typeid(int64_t)) {
      int64_t* tile_extents_int64 = (int64_t*) tile_extents_;
      for(int i=0; i<dim_num_; ++i)
        std::cout << "\t" << dimensions_[i] << ": " << tile_extents_int64[i] << "\n";
    } else if(types_[attribute_num_] == &typeid(float)) {
      float* tile_extents_float = (float*) tile_extents_;
      for(int i=0; i<dim_num_; ++i)
        std::cout << "\t" << dimensions_[i] << ": " << tile_extents_float[i] << "\n";
    } else if(types_[attribute_num_] == &typeid(double)) {
      double* tile_extents_double = (double*) tile_extents_;
      for(int i=0; i<dim_num_; ++i)
        std::cout << "\t" << dimensions_[i] << ": " << tile_extents_double[i] << "\n";
    }
  }
  // Consolidation step
  std::cout << "Consolidation step:\n\t" << consolidation_step_ << "\n";
  // Compression type
  std::cout << "Compression type:\n";
  for(int i=0; i<attribute_num_; ++i)
    if(compression_[i] == TILEDB_AS_CMP_GZIP)
      std::cout << "\t" << attributes_[i] << ": GZIP\n";
    else if(compression_[i] == TILEDB_AS_CMP_NONE)
      std::cout << "\t" << attributes_[i] << ": NONE\n";
  if(compression_[attribute_num_] == TILEDB_AS_CMP_GZIP)
    std::cout << "\tCoordinates: GZIP\n";
  else if(compression_[attribute_num_] == TILEDB_AS_CMP_NONE)
    std::cout << "\tCoordinates: NONE\n";
}

// ===== FORMAT =====
// array_name_size(int) 
//     array_name(string)
// dense(bool)
// key_value(bool)
// tile_order(char)
// cell_order(char)
// capacity(int64_t)
// consolidation_step(int)
// attribute_num(int) 
//     attribute_size#1(int) attribute#1(string)
//     attribute_size#2(int) attribute#2(string) 
//     ...
// dim_num(int) 
//    dimension_size#1(int) dimension#1(string)
//    dimension_size#2(int) dimension#2(string)
//    ...
// domain_size(int)
// domain#1_low(double) dim_domain#1_high(double)
// domain#2_low(double) dim_domain#2_high(double)
//  ...
// tile_extents_size(int) 
//     tile_extent#1(double) tile_extent#2(double) ... 
// type#1(char) type#2(char) ... 
// val_num#1(int) val_num#2(int) ... 
// compression#1(char) compression#2(char) ...
int ArraySchema::serialize(
    void*& array_schema_bin,
    size_t& array_schema_bin_size) const {
  // Compute the size of the binary representation of the ArraySchema object
  array_schema_bin_size = compute_bin_size();

  // Allocate memory
  array_schema_bin = malloc(array_schema_bin_size);

  // For easy reference
  char* buffer = (char*) array_schema_bin;
  size_t buffer_size = array_schema_bin_size;
  size_t offset = 0;

  // Copy array_name_
  int array_name_size = array_name_.size();
  assert(offset + sizeof(int) < buffer_size);
  memcpy(buffer + offset, &array_name_size, sizeof(int));
  offset += sizeof(int);
  assert(offset + array_name_size < buffer_size);
  memcpy(buffer + offset, &array_name_[0], array_name_size);
  offset += array_name_size;
  // Copy dense_
  assert(offset + sizeof(bool) < buffer_size);
  memcpy(buffer + offset, &dense_, sizeof(bool));
  offset += sizeof(bool);
  // Copy key_value_
  assert(offset + sizeof(bool) < buffer_size);
  memcpy(buffer + offset, &key_value_, sizeof(bool));
  offset += sizeof(bool);
  // Copy tile_order_
  char tile_order = tile_order_;
  assert(offset + sizeof(char) < buffer_size);
  memcpy(buffer + offset, &tile_order, sizeof(char));
  offset += sizeof(char);
  // Copy cell_order_
  char cell_order = cell_order_;
  assert(offset + sizeof(char) < buffer_size);
  memcpy(buffer + offset, &cell_order, sizeof(char));
  offset += sizeof(char);
  // Copy capacity_
  assert(offset + sizeof(int64_t) < buffer_size);
  memcpy(buffer + offset, &capacity_, sizeof(int64_t));
  offset += sizeof(int64_t);
  // Copy consolidation_step_
  assert(offset + sizeof(int) < buffer_size);
  memcpy(buffer + offset, &consolidation_step_, sizeof(int));
  offset += sizeof(int);
  // Copy attributes_
  assert(offset + sizeof(int) < buffer_size);
  memcpy(buffer + offset, &attribute_num_, sizeof(int));
  offset += sizeof(int);
  int attribute_size;
  for(int i=0; i<attribute_num_; i++) {
    attribute_size = attributes_[i].size();
    assert(offset + sizeof(int) < buffer_size);
    memcpy(buffer + offset, &attribute_size, sizeof(int)); 
    offset += sizeof(int);
    assert(offset + attribute_size < buffer_size);
    memcpy(buffer + offset, attributes_[i].c_str(), attribute_size); 
    offset += attribute_size;
  }
  // Copy dimensions_
  assert(offset + sizeof(int) < buffer_size);
  memcpy(buffer + offset, &dim_num_, sizeof(int));
  offset += sizeof(int);
  int dimension_size;
  for(int i=0; i<dim_num_; i++) {
    dimension_size = dimensions_[i].size();
    assert(offset + sizeof(int) < buffer_size);
    memcpy(buffer + offset, &dimension_size, sizeof(int)); 
    offset += sizeof(int);
    assert(offset + dimension_size < buffer_size);
    memcpy(buffer + offset, dimensions_[i].c_str(), dimension_size); 
    offset += dimension_size;
  }
  // Copy domain_
  int domain_size = 2*coords_size();
  assert(offset + sizeof(int) < buffer_size);
  memcpy(buffer + offset, &domain_size, sizeof(int));
  offset += sizeof(int);
  assert(offset + domain_size < buffer_size);
  memcpy(buffer + offset, domain_, domain_size);
  offset += 2*coords_size();
  // Copy tile_extents_
  int tile_extents_size = ((tile_extents_ == NULL) ? 0 : coords_size()); 
  assert(offset + sizeof(int) < buffer_size);
  memcpy(buffer + offset, &tile_extents_size, sizeof(int));
  offset += sizeof(int);
  if(tile_extents_ != NULL) {
    assert(offset + tile_extents_size < buffer_size);
    memcpy(buffer + offset, tile_extents_, tile_extents_size);
    offset += tile_extents_size;
  }
  // Copy types_
  char type; 
  for(int i=0; i<=attribute_num_; i++) {
    if(*types_[i] == typeid(char))
      type = TILEDB_CHAR;
    else if(*types_[i] == typeid(int))
      type = TILEDB_INT32;
    else if(*types_[i] == typeid(int64_t))
      type = TILEDB_INT64;
    else if(*types_[i] == typeid(float))
      type = TILEDB_FLOAT32;
    else if(*types_[i] == typeid(double))
      type = TILEDB_FLOAT64;
    assert(offset + sizeof(char) < buffer_size);
    memcpy(buffer + offset, &type, sizeof(char));
    offset += sizeof(char);
  }
  // Copy val_num_
  for(int i=0; i<attribute_num_; i++) {
    assert(offset + sizeof(int) < buffer_size);
    memcpy(buffer + offset, &val_num_[i], sizeof(int));
    offset += sizeof(int);
  }
  // Copy compression_
  char compression; 
  for(int i=0; i<=attribute_num_; ++i) {
    compression = compression_[i];
    assert(offset + sizeof(char) <= buffer_size);
    memcpy(buffer + offset, &compression, sizeof(char));
    offset += sizeof(char);
  }
  assert(offset == buffer_size);

  return TILEDB_AS_OK;
}

const void* ArraySchema::tile_domain() const {
  return tile_domain_;
}

const void* ArraySchema::tile_extents() const {
  return tile_extents_;
}

int64_t ArraySchema::tile_num() const {
  // Invoke the proper template function 
  if(types_[attribute_num_] == &typeid(int))
    return tile_num<int>();
  else if(types_[attribute_num_] == &typeid(int64_t))
    return tile_num<int64_t>();
  else
    assert(0);
}

template<class T>
int64_t ArraySchema::tile_num() const {
  // For easy reference
  const T* domain = static_cast<const T*>(domain_);
  const T* tile_extents = static_cast<const T*>(tile_extents_);

  int64_t ret = 1;
  for(int i=0; i<dim_num_; ++i) 
    ret *= (domain[2*i+1] - domain[2*i] + 1) / tile_extents[i];

  return ret; 
}

size_t ArraySchema::tile_size(int attribute_id) const {
  // Sanity checks
  assert(dense_ || tile_extents_ == NULL);

  return tile_sizes_[attribute_id];
}

const std::type_info* ArraySchema::type(int i) const {
  if(i<0 || i>attribute_num_)
    return NULL;
  else
    return types_[i];
}

bool ArraySchema::var_size(int attribute_id) const {
  return cell_sizes_[attribute_id] == TILEDB_AS_VAR_SIZE; 
}

int ArraySchema::var_attribute_num() const {
  return var_attribute_num_;
}

/* ****************************** */
/*             MUTATORS           */
/* ****************************** */

template<class T>
void ArraySchema::compute_hilbert_bits() {
  // For easy reference
  const T* domain = static_cast<const T*>(domain_);
  T max_domain_range = 0;
  T domain_range;

  for(int i = 0; i < dim_num_; ++i) { 
    domain_range = domain[2*i+1] - domain[2*i] + 1;
    if(max_domain_range < domain_range)
      max_domain_range = domain_range;
  }

  hilbert_bits_ = ceil(log2(int64_t(max_domain_range+0.5)));
}

// ===== FORMAT =====
// array_name_size(int) 
//     array_name(string)
// dense(bool)
// key_value(bool)
// tile_order(char)
// cell_order(char)
// capacity(int64_t)
// consolidation_step(int)
// attribute_num(int) 
//     attribute_size#1(int) attribute#1(string)
//     attribute_size#2(int) attribute#2(string) 
//     ...
// dim_num(int) 
//    dimension_size#1(int) dimension#1(string)
//    dimension_size#2(int) dimension#2(string)
//    ...
// domain_size(int)
// domain#1_low(double) dim_domain#1_high(double)
// domain#2_low(double) dim_domain#2_high(double)
//  ...
// tile_extents_size(int) 
//     tile_extent#1(double) tile_extent#2(double) ... 
// type#1(char) type#2(char) ... 
// val_num#1(int) val_num#2(int) ... 
// compression#1(char) compression#2(char) ...
int ArraySchema::deserialize(
    const void* array_schema_bin, 
    size_t array_schema_bin_size) {
  // For easy reference
  const char* buffer = static_cast<const char*>(array_schema_bin);
  size_t buffer_size = array_schema_bin_size;
  size_t offset = 0;

  // Load array_name_ 
  int array_name_size;
  assert(offset + sizeof(int) < buffer_size);
  memcpy(&array_name_size, buffer + offset, sizeof(int));
  offset += sizeof(int);
  array_name_.resize(array_name_size);
  assert(offset + array_name_size < buffer_size);
  memcpy(&array_name_[0], buffer + offset, array_name_size);
  offset += array_name_size;
  // Load dense_
  assert(offset + sizeof(bool) < buffer_size);
  memcpy(&dense_, buffer + offset, sizeof(bool));
  offset += sizeof(bool);
  // Load key_value_
  assert(offset + sizeof(bool) < buffer_size);
  memcpy(&key_value_, buffer + offset, sizeof(bool));
  offset += sizeof(bool);
  // Load tile_order_ 
  char tile_order;
  assert(offset + sizeof(char) < buffer_size);
  memcpy(&tile_order, buffer + offset, sizeof(char));
  tile_order_ = static_cast<TileOrder>(tile_order);  
  offset += sizeof(char);
  // Load cell_order_
  char cell_order;
  assert(offset + sizeof(char) < buffer_size);
  memcpy(&cell_order, buffer + offset, sizeof(char));
  cell_order_ = static_cast<CellOrder>(cell_order);  
  offset += sizeof(char);
  // Load capacity_
  assert(offset + sizeof(int64_t) < buffer_size);
  memcpy(&capacity_, buffer + offset, sizeof(int64_t));
  offset += sizeof(int64_t);
  // Load consolidation_step_
  assert(offset + sizeof(int) < buffer_size);
  memcpy(&consolidation_step_, buffer + offset, sizeof(int));
  offset += sizeof(int);
  // Load attributes_
  assert(offset + sizeof(int) < buffer_size);
  memcpy(&attribute_num_, buffer + offset, sizeof(int));
  offset += sizeof(int);
  attributes_.resize(attribute_num_);
  int attribute_size;
  for(int i=0; i<attribute_num_; ++i) {
    assert(offset + sizeof(int) < buffer_size);
    memcpy(&attribute_size, buffer + offset, sizeof(int)); 
    offset += sizeof(int);
    attributes_[i].resize(attribute_size);
    assert(offset + attribute_size < buffer_size);
    memcpy(&attributes_[i][0], buffer + offset, attribute_size);
    offset += attribute_size;
  }
  // Load dimensions_
  assert(offset + sizeof(int) < buffer_size);
  memcpy(&dim_num_, buffer + offset, sizeof(int));
  offset += sizeof(int);
  dimensions_.resize(dim_num_);
  int dimension_size;
  for(int i=0; i<dim_num_; ++i) {
    assert(offset + sizeof(int) < buffer_size);
    memcpy(&dimension_size, buffer + offset, sizeof(int)); 
    offset += sizeof(int);
    dimensions_[i].resize(dimension_size);
    assert(offset  + dimension_size < buffer_size);
    memcpy(&dimensions_[i][0], buffer + offset, dimension_size); 
    offset += dimension_size;
  }
  // Load domain_
  int domain_size;
  assert(offset + sizeof(int) < buffer_size);
  memcpy(&domain_size, buffer + offset, sizeof(int));
  offset += sizeof(int);
  assert(offset + domain_size < buffer_size);
  domain_ = malloc(domain_size);
  memcpy(domain_, buffer + offset, domain_size); 
  offset += domain_size;
  // Load tile_extents_
  int tile_extents_size;
  assert(offset + sizeof(int) < buffer_size);
  memcpy(&tile_extents_size, buffer + offset, sizeof(int));
  offset += sizeof(int);
  if(tile_extents_size == 0) {
    tile_extents_ = NULL;
  } else {
    assert(offset + tile_extents_size < buffer_size);
    tile_extents_ = malloc(tile_extents_size); 
    memcpy(tile_extents_, buffer + offset, tile_extents_size);
    offset += tile_extents_size;
  }
  // Load types_ and set type sizes
  char type;
  types_.resize(attribute_num_+1); 
  type_sizes_.resize(attribute_num_+1);
  for(int i=0; i<=attribute_num_; ++i) {
    assert(offset + sizeof(char) < buffer_size);
    memcpy(&type, buffer + offset, sizeof(char));
    offset += sizeof(char);
    if(type == TILEDB_CHAR) {
      types_[i] = &typeid(char);
      type_sizes_[i] = sizeof(char);
    } else if(type == TILEDB_INT32) {
      types_[i] = &typeid(int);
      type_sizes_[i] = sizeof(int);
    } else if(type == TILEDB_INT64) {
      types_[i] = &typeid(int64_t);
      type_sizes_[i] = sizeof(int64_t);
    } else if(type == TILEDB_FLOAT32) {
      types_[i] = &typeid(float);
      type_sizes_[i] = sizeof(float);
    } else if(type == TILEDB_FLOAT64) {
      types_[i] = &typeid(double);
      type_sizes_[i] = sizeof(double);
    }
  }
  // Load val_num_
  var_attribute_num_ = 0;
  val_num_.resize(attribute_num_); 
  for(int i=0; i<attribute_num_; ++i) {
    assert(offset + sizeof(int) < buffer_size);
    memcpy(&val_num_[i], buffer + offset, sizeof(int));
    if(val_num_[i] == TILEDB_AS_VAR_SIZE)
      ++var_attribute_num_;
    offset += sizeof(int);
  }
  // Load compression_
  char compression;
  for(int i=0; i<=attribute_num_; ++i) {
    assert(offset + sizeof(char) <= buffer_size);
    memcpy(&compression, buffer + offset, sizeof(char));
    offset += sizeof(char);
    compression_.push_back(static_cast<Compression>(compression));
  }
  assert(offset == buffer_size); 
  // Add extra coordinate attribute
  attributes_.push_back(TILEDB_COORDS_NAME);
  // Set cell sizes
  cell_sizes_.resize(attribute_num_+1);
  for(int i=0; i<= attribute_num_; ++i) 
    cell_sizes_[i] = compute_cell_size(i);

  // Compute number of cells per tile
  compute_cell_num_per_tile();

  // Compute tile sizes
  compute_tile_sizes();

  // Compute tile domain
  compute_tile_domain();

  // Initialize Hilbert curve
  init_hilbert_curve();

  return TILEDB_AS_OK;
}

int ArraySchema::init(const ArraySchemaC* array_schema_c) {
  // Set array name
  set_array_name(array_schema_c->array_name_);
  // Set attributes
  if(set_attributes(
      array_schema_c->attributes_, 
      array_schema_c->attribute_num_) != TILEDB_AS_OK)
    return TILEDB_AS_ERR;
  // Set capacity
  set_capacity(array_schema_c->capacity_);
  // Set dimensions
  if(set_dimensions(
      array_schema_c->dimensions_, 
      array_schema_c->dim_num_) != TILEDB_AS_OK)
    return TILEDB_AS_ERR;
  // Set compression
  if(set_compression(array_schema_c->compression_) != TILEDB_AS_OK)
    return TILEDB_AS_ERR;
  // Set consolidation step
  set_consolidation_step(array_schema_c->consolidation_step_);
  // Set dense
  set_dense(array_schema_c->dense_);
  // Set types
  if(set_types(array_schema_c->types_) != TILEDB_AS_OK)
    return TILEDB_AS_ERR;
  // Set tile extents
  if(set_tile_extents(array_schema_c->tile_extents_) != TILEDB_AS_OK)
    return TILEDB_AS_ERR;
  // Set cell order
  if(set_cell_order(array_schema_c->cell_order_) != TILEDB_AS_OK)
    return TILEDB_AS_ERR;
  // Set tile order
  if(set_tile_order(array_schema_c->tile_order_) != TILEDB_AS_OK)
    return TILEDB_AS_ERR;
  // Set domain
  if(set_domain(array_schema_c->domain_) != TILEDB_AS_OK)
    return TILEDB_AS_ERR;

  // Compute number of cells per tile
  compute_cell_num_per_tile();

  // Compute tile sizes
  compute_tile_sizes();

  // Compute tile domain
  compute_tile_domain();

  // Initialize Hilbert curve
  init_hilbert_curve();

  return TILEDB_AS_OK;
}

void ArraySchema::init_hilbert_curve() {
  // Applicable only to Hilbert cell order
  if(cell_order_ != TILEDB_AS_CO_HILBERT) 
    return;

  // Allocate some space for the Hilbert coordinates
  if(coords_for_hilbert_ == NULL)
    coords_for_hilbert_ = new int[dim_num_];

  // Compute Hilbert bits, invoking the proper templated function
  if(types_[attribute_num_] == &typeid(int))
    compute_hilbert_bits<int>();
  else if(types_[attribute_num_] == &typeid(int64_t))
    compute_hilbert_bits<int64_t>();
  else if(types_[attribute_num_] == &typeid(float))
    compute_hilbert_bits<float>();
  else if(types_[attribute_num_] == &typeid(double))
    compute_hilbert_bits<double>();

  // Create new Hilberrt curve
  hilbert_curve_ = new HilbertCurve(hilbert_bits_, dim_num_);
}

void ArraySchema::set_array_name(const char* array_name) {
  // Get real array name
  std::string array_name_real = real_dir(array_name);

  // Set array name
  array_name_ = array_name_real;
}

int ArraySchema::set_attributes(
    const char** attributes,
    int attribute_num) {
  // Sanity check on attributes
  if(attributes == NULL) {
    PRINT_ERROR("Cannot set attributes; No attributes given");
    return TILEDB_AS_ERR;
  }

  // Sanity check on attribute number
  if(attribute_num <= 0) {
    PRINT_ERROR("Cannot set attributes; "
                "The number of attributes must be positive");
    return TILEDB_AS_ERR;
  }

  // Set attributes and attribute number
  for(int i=0; i<attribute_num; ++i) 
    attributes_.push_back(attributes[i]);
  attribute_num_ = attribute_num;

  // Append extra coordinates name
  attributes_.push_back(TILEDB_COORDS_NAME);

  // Check for duplicate attribute names
  if(has_duplicates(attributes_)) {
    PRINT_ERROR("Cannot set attributes; Duplicate attribute names");
    return TILEDB_AS_ERR;
  }

  // Check if a dimension has the same name as an attribute 
  if(intersect(attributes_, dimensions_)) {
    PRINT_ERROR("Cannot set attributes; Attribute name same as dimension name");
    return TILEDB_AS_ERR;
  }

  return TILEDB_AS_OK;
}

void ArraySchema::set_capacity(int capacity) {
  // Set capacity
  if(capacity > 0)
    capacity_ = capacity;
  else
    capacity_ = TILEDB_AS_CAPACITY;
}

int ArraySchema::set_cell_order(const char* cell_order) {
  // Set cell order
  if(cell_order == NULL) {
    cell_order_ = TILEDB_AS_CELL_ORDER;
  } else if(!strcmp(cell_order, "row-major")) {
    cell_order_ = TILEDB_AS_CO_ROW_MAJOR;
  } else if(!strcmp(cell_order, "column-major")) {
    cell_order_ = TILEDB_AS_CO_COLUMN_MAJOR;
  } else if(!strcmp(cell_order, "hilbert")) {
    if(tile_extents_ != NULL) {
      PRINT_ERROR("Cannot set cell order; Arrays with non-null tile extents do "
                  "not support hilbert order");
      return TILEDB_AS_ERR;
    }
    cell_order_ = TILEDB_AS_CO_HILBERT;
  } else {
    PRINT_ERROR(std::string("Cannot set cell order; Invalid cell order '") + 
                cell_order + "'");
    return TILEDB_AS_ERR;
  }

  return TILEDB_AS_OK;
}

int ArraySchema::set_compression(const char** compression) {
  // Set compression  
  if(compression == NULL) {
    for(int i=0; i<attribute_num_+1; ++i)
      compression_.push_back(TILEDB_AS_CMP_NONE);
  } else {
    for(int i=0; i<attribute_num_+1; ++i) {
      if(!strcmp(compression[i], "NONE")) { 
        compression_.push_back(TILEDB_AS_CMP_NONE);
      } else if(!strcmp(compression[i], "GZIP")) {
        compression_.push_back(TILEDB_AS_CMP_GZIP);
      } else {
        PRINT_ERROR(std::string("Cannot set compression; "
                    "Invalid compression type '") + 
                    compression[i] + "'");
        return TILEDB_AS_ERR;
      }
    }
  }

  return TILEDB_AS_OK;
}

void ArraySchema::set_consolidation_step(int consolidation_step) {
  // Set consilidation step
  if(consolidation_step > 0)
    consolidation_step_ = consolidation_step;
  else
    consolidation_step_ = TILEDB_AS_CONSOLIDATION_STEP;
}

void ArraySchema::set_dense(int dense) {
  dense_ = dense;
}

int ArraySchema::set_dimensions(
    const char** dimensions,
    int dim_num) {
  // Sanity check on dimensions
  if(dimensions == NULL) {
    PRINT_ERROR("Cannot set dimensions; No dimensions given");
    return TILEDB_AS_ERR;
  }

  // Sanity check on dimension number
  if(dim_num <= 0) {
    PRINT_ERROR("Cannot set dimensions; "
                "The number of dimensions must be positive");
    return TILEDB_AS_ERR;
  }

  // Set dimensions and dimension number
  for(int i=0; i<dim_num; ++i) 
    dimensions_.push_back(dimensions[i]);
  dim_num_ = dim_num;

  // Check for duplicate dimension names
  if(has_duplicates(dimensions_)) {
    PRINT_ERROR("Cannot set dimensions; Duplicate dimension names");
    return TILEDB_AS_ERR;
  }

  // Check if a dimension has the same name as an attribute 
  if(intersect(attributes_, dimensions_)) {
    PRINT_ERROR("Cannot set dimensions; Attribute name same as dimension name");
    return TILEDB_AS_ERR;
  }

  return TILEDB_AS_OK;
}

int ArraySchema::set_domain(const void* domain) {
  // Sanity check
  if(domain == NULL) {
    PRINT_ERROR("Cannot set domain; Domain not provided");
    return TILEDB_AS_ERR;
  }

  // Clear domain
  if(domain_ != NULL)
    free(domain_);

  // Set domain
  size_t domain_size = 2*coords_size();
  domain_ = malloc(domain_size);
  memcpy(domain_, domain, domain_size);

  // Sanity check
  if(types_[attribute_num_] == &typeid(int)) {
    int* domain_int = (int*) domain_;
    for(int i=0; i<dim_num_; ++i) {
      if(domain_int[2*i] > domain_int[2*i+1]) {
        PRINT_ERROR("Cannot set domain; Lower domain bound larger than its "
                    "corresponding upper");
        return TILEDB_AS_ERR;
      }
    }
  } else if(types_[attribute_num_] == &typeid(int64_t)) {
    int64_t* domain_int64 = (int64_t*) domain_;
    for(int i=0; i<dim_num_; ++i) {
      if(domain_int64[2*i] > domain_int64[2*i+1]) {
        PRINT_ERROR("Cannot set domain; Lower domain bound larger than its "
                    "corresponding upper");
        return TILEDB_AS_ERR;
      }
    }
  } else if(types_[attribute_num_] == &typeid(float)) {
    float* domain_float = (float*) domain_;
    for(int i=0; i<dim_num_; ++i) {
      if(domain_float[2*i] > domain_float[2*i+1]) {
        PRINT_ERROR("Cannot set domain; Lower domain bound larger than its "
                    "corresponding upper");
        return TILEDB_AS_ERR;
      }
    }
  } else if(types_[attribute_num_] == &typeid(double)) {
    double* domain_double = (double*) domain_;
    for(int i=0; i<dim_num_; ++i) {
      if(domain_double[2*i] > domain_double[2*i+1]) {
        PRINT_ERROR("Cannot set domain; Lower domain bound larger than its "
                    "corresponding upper");
        return TILEDB_AS_ERR;
      }
    }
  } else {
    PRINT_ERROR("Cannot set domain; Invalid coordinates type");
    return TILEDB_AS_ERR;
  }

  return TILEDB_AS_OK;
}

int ArraySchema::set_tile_extents(const void* tile_extents) {
  // Dense arrays must have tile extents
  if(tile_extents == NULL && dense_) {
    PRINT_ERROR("Cannot set tile extents; Dense arrays must have tile extents");
    return TILEDB_AS_ERR;
  }

  // Set tile extents
  if(tile_extents_ != NULL) {
    // Free existing tile extends
    free(tile_extents_);
  }

  // Set tile extents
  if(tile_extents == NULL) {
    tile_extents_ = NULL;
  } else { 
    size_t tile_extents_size = coords_size();
    tile_extents_ = malloc(tile_extents_size); 
    memcpy(tile_extents_, tile_extents, tile_extents_size);
  }

  return TILEDB_AS_OK;
}

int ArraySchema::set_tile_order(const char* tile_order) {
  // Set tile order
  if(tile_order == NULL) {
    tile_order_ = TILEDB_AS_TILE_ORDER;
  } else if(!strcmp(tile_order, "row-major")) {
    tile_order_ = TILEDB_AS_TO_ROW_MAJOR;
  } else if(!strcmp(tile_order, "column-major")) {
    tile_order_ = TILEDB_AS_TO_COLUMN_MAJOR;
  } else if(!strcmp(tile_order, "hilbert")) {
    if(tile_extents_ != NULL) {
      PRINT_ERROR("Cannot set tile order; Arrays with non-null tile extents do "
                  "not support hilbert order");
      return TILEDB_AS_ERR;
    }
    tile_order_ = TILEDB_AS_TO_HILBERT;
  } else {
    PRINT_ERROR(std::string("Cannot set tile order; Invalid tile order '") + 
                tile_order + "'");
    return TILEDB_AS_ERR;
  }

  return TILEDB_AS_OK;
}

int ArraySchema::set_types(const char** types) {
  // Sanity check
  if(types == NULL) {
    PRINT_ERROR("Cannot set types; Types not provided");
    return TILEDB_AS_ERR;
  }

  // Set attribute types
  std::string type_val_num;
  std::string type;
  int num;
  var_attribute_num_ = 0;

  for(int i=0; i<attribute_num_; ++i) { 
    type_val_num = types[i];
    char* token = strtok(&type_val_num[0], ":"); // Type extraction
    type = token;    
    token = strtok(NULL, ":"); // Number of attribute values per cell extraction
    if(token == NULL) { // Missing number of attribute values per cell
      val_num_.push_back(1);
    } else {
      // Process next token
      if(!strcmp(token, "var")) { // Variable-sized cell
        val_num_.push_back(TILEDB_AS_VAR_SIZE);
        ++var_attribute_num_;
      } else if(!is_positive_integer(token)) { 
        PRINT_ERROR("Cannot set types; The number of attribute values per "
                    "cell must be a positive integer");
        return TILEDB_AS_ERR;
      } else { // Valid number of attribute values per cell
        sscanf(token, "%d", &num);
        val_num_.push_back(num);
      }

      // No other token should follow
      token = strtok(NULL, ":");
      if(token != NULL) {
        PRINT_ERROR("Cannot set types; Redundant arguments");
        return TILEDB_AS_ERR;
      }
    }

    if(type == "char") {
      types_.push_back(&typeid(char));
    } else if(type == "int32") {
      types_.push_back(&typeid(int));
    } else if(type == "int64") {
      types_.push_back(&typeid(int64_t));
    } else if(type == "float32") {
      types_.push_back(&typeid(float));
    } else if(type == "float64") {
      types_.push_back(&typeid(double));
    } else {
      PRINT_ERROR(std::string("Cannot set types; Invalid attribute type '") + 
                  type + "'");
      return TILEDB_AS_ERR;
    }
  } 

  // Set coordinate type
  type = types[attribute_num_];
  if(type == "char:var") { // Key value - create 4 dimensions for the hashes
    types_.push_back(&typeid(int));
    dim_num_ = 4;
    key_value_ = true;
    std::string dimension = dimensions_.front();
    dimensions_.resize(dim_num_);
    dimensions_[0] = dimension + "_1";
    dimensions_[1] = dimension + "_2";
    dimensions_[2] = dimension + "_3";
    dimensions_[3] = dimension + "_4";
  } else {
    key_value_ = false;

    if(dense_ && (type == "float32" || type == "float64")) {
      PRINT_ERROR("Cannot set types; Dense arrays may only "
                  "have coordinates of type \"int32\" or \"int64\"");
      return TILEDB_AS_ERR;
    }

    if(type == "int32") {
      types_.push_back(&typeid(int));
    } else if(type == "int64") {
      types_.push_back(&typeid(int64_t));
    } else if(type == "float32") {
      types_.push_back(&typeid(float));
    } else if(type == "float64") {
      types_.push_back(&typeid(double));
    } else {
      PRINT_ERROR(std::string("Invalid coordinates type '") + type + "'");
      return TILEDB_AS_ERR;
    }
  }

  // Set type sizes
  type_sizes_.resize(attribute_num_ + 1);
  for(int i=0; i < attribute_num_+1; ++i)
    type_sizes_[i] = compute_type_size(i);

  // Set cell sizes
  cell_sizes_.resize(attribute_num_+1);
  for(int i=0; i < attribute_num_+1; ++i) 
    cell_sizes_[i] = compute_cell_size(i);

  return TILEDB_AS_OK;
}

/* ****************************** */
/*              MISC              */
/* ****************************** */

template<class T>
T ArraySchema::cell_num_in_range_slab(const T* range) const {
  // Invoke the proper function based on the cell order
  if(cell_order_ == TILEDB_AS_CO_ROW_MAJOR)
    return cell_num_in_range_slab_row(range);
  else if(cell_order_ == TILEDB_AS_CO_COLUMN_MAJOR)
    return cell_num_in_range_slab_col(range);
  else
    return -1;
}

template<class T>
T ArraySchema::cell_num_in_range_slab_col(const T* range) const {
  return range[1] - range[0] + 1;
}

template<class T>
T ArraySchema::cell_num_in_range_slab_row(const T* range) const {
  return range[2*(dim_num_-1)+1] - range[2*(dim_num_-1)] + 1;
}

template<class T>
T ArraySchema::cell_num_in_tile_slab() const {
  // Invoke the proper function based on the cell order
  if(cell_order_ == TILEDB_AS_CO_ROW_MAJOR)
    return cell_num_in_tile_slab_row<T>();
  else if(cell_order_ == TILEDB_AS_CO_COLUMN_MAJOR)
    return cell_num_in_tile_slab_col<T>();
  else
    return -1;
}

template<class T>
T ArraySchema::cell_num_in_tile_slab_col() const {
  // For easy reference
  const T* tile_extents = static_cast<const T*>(tile_extents_);

  return tile_extents[0];
}

template<class T>
T ArraySchema::cell_num_in_tile_slab_row() const {
  // For easy reference
  const T* tile_extents = static_cast<const T*>(tile_extents_);

  return tile_extents[dim_num_-1];
}

template<class T>
int64_t ArraySchema::get_cell_pos(const T* coords) const {
  // Invoke the proper function based on the cell order
  if(cell_order_ == TILEDB_AS_CO_ROW_MAJOR)
    return get_cell_pos_row(coords);
  else if(cell_order_ == TILEDB_AS_CO_COLUMN_MAJOR)
    return get_cell_pos_col(coords);
  else
    return -1;
}

template<class T>
int64_t ArraySchema::get_cell_pos_col(const T* coords) const {
  // For easy reference
  const T* tile_extents = static_cast<const T*>(tile_extents_);

  int64_t pos = 0;
  
  // Calculate cell offsets
  int64_t cell_num; // Per dimension
  std::vector<int64_t> cell_offsets;
  cell_offsets.push_back(1);
  for(int i=1; i<dim_num_; ++i) {
    cell_num = tile_extents[i-1]; 
    cell_offsets.push_back(cell_offsets.back() * cell_num);
  }
 
  // Calculate position
  for(int i=0; i<dim_num_; ++i) 
    pos += coords[i] * cell_offsets[i];

  return pos;
}

template<class T>
int64_t ArraySchema::get_cell_pos_row(const T* coords) const {
  // For easy reference
  const T* tile_extents = static_cast<const T*>(tile_extents_);

  int64_t pos = 0;
  
  // Calculate cell offsets
  int64_t cell_num; // Per dimension
  std::vector<int64_t> cell_offsets;
  cell_offsets.push_back(1);
  for(int i=dim_num_-2; i>=0; --i) {
    cell_num = tile_extents[i+1];
    cell_offsets.push_back(cell_offsets.back() * cell_num);
  }
  std::reverse(cell_offsets.begin(), cell_offsets.end());
 
  // Calculate position
  for(int i=0; i<dim_num_; ++i) 
    pos += coords[i] * cell_offsets[i];

  return pos;
}

template<class T>
void ArraySchema::get_next_tile_coords(
    const T* domain,
    T* tile_coords) const {
  // Invoke the proper function based on the tile order
  if(tile_order_ == TILEDB_AS_TO_ROW_MAJOR)
    get_next_tile_coords_row(domain, tile_coords);
  else if(tile_order_ == TILEDB_AS_TO_COLUMN_MAJOR)
    get_next_tile_coords_col(domain, tile_coords);
}

template<class T>
void ArraySchema::get_next_tile_coords_col(
    const T* domain,
    T* tile_coords) const {
  int i = 0;
  ++tile_coords[i];

  while(i < dim_num_-1 && tile_coords[i] > domain[2*i+1]) {
    tile_coords[i] = domain[2*i];
    ++tile_coords[++i];
  } 
}

template<class T>
void ArraySchema::get_next_tile_coords_row(
    const T* domain,
    T* tile_coords) const {
  int i = dim_num_-1;
  ++tile_coords[i];

  while(i > 0 && tile_coords[i] > domain[2*i+1]) {
    tile_coords[i] = domain[2*i];
    ++tile_coords[--i];
  } 
}

template<class T>
int64_t ArraySchema::get_tile_pos(const T* tile_coords) const {
  // Invoke the proper function based on the tile order
  if(tile_order_ == TILEDB_AS_TO_ROW_MAJOR)
    get_tile_pos_row(tile_coords);
  else if(tile_order_ == TILEDB_AS_TO_COLUMN_MAJOR)
    get_tile_pos_col(tile_coords);
}

template<class T>
int64_t ArraySchema::get_tile_pos_col(const T* tile_coords) const {
  // For easy reference
  const T* domain = static_cast<const T*>(domain_);
  const T* tile_extents = static_cast<const T*>(tile_extents_);

  int64_t pos = 0;
  
  // Calculate tile offsets
  int64_t tile_num; // Per dimension
  std::vector<int64_t> tile_offsets;
  tile_offsets.push_back(1);
  for(int i=1; i<dim_num_; ++i) {
    tile_num = (domain[2*(i-1)+1] - 
                domain[2*(i-1)] + 1) / tile_extents[i-1];
    tile_offsets.push_back(tile_offsets.back() * tile_num);
  }
 
  // Calculate position
  for(int i=0; i<dim_num_; ++i) 
    pos += tile_coords[i] * tile_offsets[i];

  return pos;
}

template<class T>
int64_t ArraySchema::get_tile_pos_row(const T* tile_coords) const {
  // For easy reference
  const T* domain = static_cast<const T*>(domain_);
  const T* tile_extents = static_cast<const T*>(tile_extents_);

  int64_t pos = 0;
  
  // Calculate tile offsets
  int64_t tile_num; // Per dimension
  std::vector<int64_t> tile_offsets;
  tile_offsets.push_back(1);
  for(int i=dim_num_-2; i>=0; --i) {
    tile_num = (domain[2*(i+1)+1] - 
                domain[2*(i+1)] + 1) / tile_extents[i+1];
    tile_offsets.push_back(tile_offsets.back() * tile_num);
  }
  std::reverse(tile_offsets.begin(), tile_offsets.end());
 
  // Calculate position
  for(int i=0; i<dim_num_; ++i) 
    pos += tile_coords[i] * tile_offsets[i];

  return pos;
}

template<class T>
void ArraySchema::compute_mbr_range_overlap(
    const T* range,
    const T* mbr,
    T* overlap_range,
    int& overlap) const {
  // Get overlap range
  for(int i=0; i<dim_num_; ++i) {
    overlap_range[2*i] = 
        std::max(mbr[2*i], range[2*i]);
    overlap_range[2*i+1] = 
        std::min(mbr[2*i+1], range[2*i+1]);
  }

  // Check overlap
  overlap = 1;
  for(int i=0; i<dim_num_; ++i) {
    if(overlap_range[2*i] > mbr[2*i+1] ||
       overlap_range[2*i+1] < mbr[2*i]) {
      overlap = 0;
      break;
    }
  }

  // Check partial overlap
  if(overlap == 1) {
    for(int i=0; i<dim_num_; ++i) {
      if(overlap_range[2*i] != mbr[2*i] ||
         overlap_range[2*i+1] != mbr[2*i+1]) {
        overlap = 2;
        break;
      }
    }
  }

  // Check contig overlap (not applicable to Hilbert order)
  if(overlap == 2 && cell_order_ != TILEDB_AS_CO_HILBERT) {
    overlap = 3;
    if(cell_order_ == TILEDB_AS_CO_ROW_MAJOR) {           // Row major
      for(int i=1; i<dim_num_; ++i) {
        if(overlap_range[2*i] != mbr[2*i] ||
           overlap_range[2*i+1] != mbr[2*i+1]) {
          overlap = 2;
          break;
        }
      }
    } else if(cell_order_ == TILEDB_AS_CO_COLUMN_MAJOR) { // Column major
      for(int i=dim_num_-2; i>=0; --i) {
        if(overlap_range[2*i] != mbr[2*i] ||
           overlap_range[2*i+1] != mbr[2*i+1]) {
          overlap = 2;
          break;
        }
      }
    }
  }
}

template<class T>
void ArraySchema::compute_tile_range_overlap(
    const T* range,
    const T* tile_coords,
    T* overlap_range,
    int& overlap) const {
  // For easy reference
  const T* domain = static_cast<const T*>(domain_);
  const T* tile_extents = static_cast<const T*>(tile_extents_);

  // Get tile range
  T* tile_range = new T[2*dim_num_];
  for(int i=0; i<dim_num_; ++i) {
    tile_range[2*i] = domain[2*i] + tile_coords[i] * tile_extents[i];
    tile_range[2*i+1] = tile_range[2*i] + tile_extents[i] - 1;
  }

  // Get overlap range
  for(int i=0; i<dim_num_; ++i) {
    overlap_range[2*i] = 
        std::max(tile_range[2*i], range[2*i]) - tile_range[2*i];
    overlap_range[2*i+1] = 
        std::min(tile_range[2*i+1], range[2*i+1]) - tile_range[2*i];
  }

  // Check overlap
  overlap = 1;
  for(int i=0; i<dim_num_; ++i) {
    if(overlap_range[2*i] >= tile_extents[i] ||
       overlap_range[2*i+1] < 0) {
      overlap = 0;
      break;
    }
  }

  // Check contig overlap
  if(overlap > 0) {
    for(int i=0; i<dim_num_; ++i) {
      if(overlap_range[2*i] != 0 ||
         overlap_range[2*i+1] != tile_extents[i] - 1) {
        overlap = 2;
        break;
      }
    }
  }

  // Check special overlap
  if(overlap == 2) {
    overlap = 3;
    if(cell_order_ == TILEDB_AS_CO_ROW_MAJOR) {           // Row major
      for(int i=1; i<dim_num_; ++i) {
        if(overlap_range[2*i] != 0 ||
           overlap_range[2*i+1] != tile_extents[i] - 1) {
          overlap = 2;
          break;
        }
      }
    } else if(cell_order_ == TILEDB_AS_CO_COLUMN_MAJOR) { // Column major
      for(int i=dim_num_-2; i>=0; --i) {
        if(overlap_range[2*i] != 0 ||
           overlap_range[2*i+1] != tile_extents[i] - 1) {
          overlap = 2;
          break;
        }
      }
    }
  }
  
  // Clean up
  delete tile_range;
}

template<typename T>
int64_t ArraySchema::hilbert_id(const T* coords) const {
  // For easy reference
  const T* domain = static_cast<const T*>(domain_);

  for(int i = 0; i < dim_num_; ++i) 
    coords_for_hilbert_[i] = static_cast<int>(coords[i] - domain[2*i]);

  int64_t id;
  hilbert_curve_->coords_to_hilbert(coords_for_hilbert_, id);

  return id;
}

/* ****************************** */
/*         PRIVATE METHODS        */
/* ****************************** */

// ===== FORMAT =====
// array_name_size(int)
//     array_name(string)
// dense(bool)
// key_value(bool)
// tile_order(char)
// cell_order(char)
// capacity(int64_t)
// consolidation_step(int)
// attribute_num(int) 
//     attribute_size#1(int) attribute#1(string)
//     attribute_size#2(int) attribute#2(string) 
//     ...
// dim_num(int) 
//    dimension_size#1(int) dimension#1(string)
//    dimension_size#2(int) dimension#2(string)
//    ...
// domain_size(int)
// domain#1_low(coordinates type) dim_domain#1_high(coordinates type)
// domain#2_low(coordinates type) dim_domain#2_high(coordinates type)
//  ...
// tile_extents_size(int) 
//     tile_extent#1(coordinates type) tile_extent#2(coordinates type) ... 
// type#1(char) type#2(char) ... 
// val_num#1(int) val_num#2(int) ... 
// compression#1(char) compression#2(char) ...
size_t ArraySchema::compute_bin_size() const {
  // Initialization
  size_t bin_size = 0;

  // Size for array_name_ 
  bin_size += sizeof(int) + array_name_.size();
  // Size for dense_
  bin_size += sizeof(bool);
  // Size for key_value_
  bin_size += sizeof(bool);
  // Size for tile_order_ and cell_order_
  bin_size += 2 * sizeof(char);
  // Size for capacity_ 
  bin_size += sizeof(int64_t);
  // Size for consolidation_step__ 
  bin_size += sizeof(int);
  // Size for attribute_num_ and attributes_ 
  bin_size += sizeof(int);
  for(int i=0; i<attribute_num_; ++i)
    bin_size += sizeof(int) + attributes_[i].size();
  // Size for dim_num and dimensions_
  bin_size += sizeof(int);
  for(int i=0; i<dim_num_; ++i)
    bin_size += sizeof(int) + dimensions_[i].size();
  // Size for domain_
  bin_size += sizeof(int) + 2*coords_size();
  // Size for tile_extents_ 
  bin_size += sizeof(int) + ((tile_extents_ == NULL) ? 0 : coords_size()); 
  // Size for types_
  bin_size += (attribute_num_+1) * sizeof(char);
  // Size for val_num_
  bin_size += attribute_num_ * sizeof(int);
  // Size for compression_
  bin_size += (attribute_num_+1) * sizeof(char);

  return bin_size;
}

void ArraySchema::compute_cell_num_per_tile() {
  if(dense_) {  // Dense
    // Invoke the proper templated function
    const std::type_info* coords_type = types_[attribute_num_];
    if(coords_type == &typeid(int)) {
      compute_cell_num_per_tile<int>();
    } else if(coords_type == &typeid(int64_t)) {
      compute_cell_num_per_tile<int64_t>();
    } else {
      // It should never reach here
      assert(0); 
    }
  } else {      // Sparse
    // Not applicable to regular tiles
    if(tile_extents_ != NULL)
      return;
    
    // The capacity is essentially the number of cells per tile
    cell_num_per_tile_ = capacity_; 
  }
}

template<class T>
void ArraySchema::compute_cell_num_per_tile() {
  if(tile_extents_ == NULL)
    return;

  const T* tile_extents = static_cast<const T*>(tile_extents_);
  cell_num_per_tile_ = 1;

  for(int i=0; i<dim_num_; ++i)
    cell_num_per_tile_ *= tile_extents[i]; 
}

size_t ArraySchema::compute_cell_size(int i) const {
  assert(i>= 0 && i <= attribute_num_);

  // Variable-sized cell
  if(i<attribute_num_ && val_num_[i] == TILEDB_AS_VAR_SIZE)
    return TILEDB_AS_VAR_SIZE;

  // Fixed-sized cell
  size_t size;
  
  // Attributes
  if(i < attribute_num_) {
    if(types_[i] == &typeid(char))
      size = val_num_[i] * sizeof(char);
    else if(types_[i] == &typeid(int))
      size = val_num_[i] * sizeof(int);
    else if(types_[i] == &typeid(int64_t))
      size = val_num_[i] * sizeof(int64_t);
    else if(types_[i] == &typeid(float))
      size = val_num_[i] * sizeof(float);
    else if(types_[i] == &typeid(double))
      size = val_num_[i] * sizeof(double);
  } else { // Coordinates
    if(types_[i] == &typeid(int))
      size = dim_num_ * sizeof(int);
    else if(types_[i] == &typeid(int64_t))
      size = dim_num_ * sizeof(int64_t);
    else if(types_[i] == &typeid(float))
      size = dim_num_ * sizeof(float);
    else if(types_[i] == &typeid(double))
      size = dim_num_ * sizeof(double);
  }

  return size; 
}

void ArraySchema::compute_tile_domain() {
  // For easy reference 
  const std::type_info* coords_type = types_[attribute_num_];

  // Invoke the proper templated function
  if(coords_type == &typeid(int))
    compute_tile_domain<int>();
  else if(coords_type == &typeid(int64_t))
    compute_tile_domain<int64_t>();
  else if(coords_type == &typeid(float))
    compute_tile_domain<float>();
  else if(coords_type == &typeid(double))
    compute_tile_domain<double>();
}

template<class T>
void ArraySchema::compute_tile_domain() {
  if(tile_extents_ == NULL)
    return;  

  // For easy reference
  const T* domain = static_cast<const T*>(domain_);
  const T* tile_extents = static_cast<const T*>(tile_extents_);

  // Allocate space for the tile domain
  assert(tile_domain_ == NULL);
  tile_domain_ = malloc(2*dim_num_*sizeof(T));

  // For easy reference
  T* tile_domain = static_cast<T*>(tile_domain_);
  T tile_num; // Per dimension

  // Calculate tile domain
  for(int i=0; i<dim_num_; ++i) {
    tile_num = ceil(double(domain[2*i+1] - domain[2*i] + 1) / tile_extents[i]);
    tile_domain[2*i] = 0;
    tile_domain[2*i+1] = tile_num - 1;
  }
}

void ArraySchema::compute_tile_sizes() {
  tile_sizes_.resize(attribute_num_ + 1);

  for(int i=0; i<attribute_num_+1; ++i) {
    if(var_size(i))
      tile_sizes_[i] = cell_num_per_tile_ * TILEDB_CELL_VAR_OFFSET_SIZE;
    else
      tile_sizes_[i] = cell_num_per_tile_ * cell_size(i); 
  }
}

size_t ArraySchema::compute_type_size(int i) const {
  assert(i>= 0 && i <= attribute_num_);

  if(types_[i] == &typeid(char))
    return sizeof(char);
  else if(types_[i] == &typeid(int))
    return sizeof(int);
  else if(types_[i] == &typeid(int64_t))
    return sizeof(int64_t);
  else if(types_[i] == &typeid(float))
    return sizeof(float);
  else if(types_[i] == &typeid(double))
    return sizeof(double);
}

// Explicit template instantiations
template int ArraySchema::cell_num_in_range_slab(
    const int* range) const;
template int64_t ArraySchema::cell_num_in_range_slab(
    const int64_t* range) const;
template float ArraySchema::cell_num_in_range_slab(
    const float* range) const;
template double ArraySchema::cell_num_in_range_slab(
    const double* range) const;

template int ArraySchema::cell_num_in_tile_slab() const;
template int64_t ArraySchema::cell_num_in_tile_slab() const;
template float ArraySchema::cell_num_in_tile_slab() const;
template double ArraySchema::cell_num_in_tile_slab() const;

template void ArraySchema::get_next_tile_coords<int>(
    const int* domain,
    int* tile_coords) const;
template void ArraySchema::get_next_tile_coords<int64_t>(
    const int64_t* domain,
    int64_t* tile_coords) const;
template void ArraySchema::get_next_tile_coords<float>(
    const float* domain,
    float* tile_coords) const;
template void ArraySchema::get_next_tile_coords<double>(
    const double* domain,
    double* tile_coords) const;

template int64_t ArraySchema::get_tile_pos<int>(
    const int* tile_coords) const;
template int64_t ArraySchema::get_tile_pos<int64_t>(
    const int64_t* tile_coords) const;
template int64_t ArraySchema::get_tile_pos<float>(
    const float* tile_coords) const;
template int64_t ArraySchema::get_tile_pos<double>(
    const double* tile_coords) const;

template void ArraySchema::compute_mbr_range_overlap<int>(
    const int* range,
    const int* mbr,
    int* overlap_range,
    int& overlap) const;
template void ArraySchema::compute_mbr_range_overlap<int64_t>(
    const int64_t* range,
    const int64_t* mbr,
    int64_t* overlap_range,
    int& overlap) const;
template void ArraySchema::compute_mbr_range_overlap<float>(
    const float* range,
    const float* mbr,
    float* overlap_range,
    int& overlap) const;
template void ArraySchema::compute_mbr_range_overlap<double>(
    const double* range,
    const double* mbr,
    double* overlap_range,
    int& overlap) const;

template void ArraySchema::compute_tile_range_overlap<int>(
    const int* range,
    const int* tile_coords,
    int* overlap_range,
    int& overlap) const;
template void ArraySchema::compute_tile_range_overlap<int64_t>(
    const int64_t* range,
    const int64_t* tile_coords,
    int64_t* overlap_range,
    int& overlap) const;
template void ArraySchema::compute_tile_range_overlap<float>(
    const float* range,
    const float* tile_coords,
    float* overlap_range,
    int& overlap) const;
template void ArraySchema::compute_tile_range_overlap<double>(
    const double* range,
    const double* tile_coords,
    double* overlap_range,
    int& overlap) const;

template int64_t ArraySchema::get_cell_pos<int>(
    const int* coords) const;
template int64_t ArraySchema::get_cell_pos<int64_t>(
    const int64_t* coords) const;
template int64_t ArraySchema::get_cell_pos<float>(
    const float* coords) const;
template int64_t ArraySchema::get_cell_pos<double>(
    const double* coords) const;

template int64_t ArraySchema::hilbert_id<int>(
    const int* coords) const;
template int64_t ArraySchema::hilbert_id<int64_t>(
    const int64_t* coords) const;
template int64_t ArraySchema::hilbert_id<float>(
    const float* coords) const;
template int64_t ArraySchema::hilbert_id<double>(
    const double* coords) const;