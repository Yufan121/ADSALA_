#include <iostream>
#include <string>
#include <math.h>
#include "domain-files.h"
#include "rapidcsv.h"
#include <algorithm> // for shuffle entries
#include <random>

//double precision of float precision
int isDouble;

/**
 * @brief get memory footprint (in Bytes)
 * 
 * @param i 
 * @param j 
 * @param k 
 * @return unsigned long long 
 */
unsigned long long mem_footprint(int i, int j, int k) {
  return (((unsigned long long)i)*j + i*k + j*k)*sizeof(float)*(isDouble + 1);//double needs double space
}

/**
 * @brief get rounded 2^x value
 * 
 * @param x 
 * @return int 
 */
int rexp2(double x) {
  return round(pow(2,x));
}

/**
 * @brief Generate samples evenly from the 3D parameter space, given the size range (in MB) and step size for sampling(on one dimension)
 * 
 * @param argc 6
 * @param argv min_size, max_size (in MB), random seed, filename, sample stepsize, isDouble (0 or 1)
 * @return int 
 */
int main(int argc, char *argv[]) {
  if (argc != 7) {
      std::cout << "Expected 6 parameters: min_footprint, max_foortprint, seed, filename, points in one dimension, isDouble" << std::endl;
      return -1;
  }

  std::stringstream ss(argv[1]);
  size_t min_footprint_;//in MB
  ss >> min_footprint_;


  std::stringstream ss1(argv[2]);
  size_t max_footprint_;//in MB
  ss1 >> max_footprint_;

  unsigned long long min_footprint=(unsigned long long)min_footprint_* 1000000;
  unsigned long long max_footprint=(unsigned long long)max_footprint_* 1000000;//convert to Byte

  int seed = std::stoi(argv[3]);
  std::string filename = argv[4];
  int points1d = std::atoi(argv[5]);
  isDouble = std::atoi(argv[6]);


  DomainFile<3> df({"m", "k", "n"});
  std::ofstream out(filename);
  srand(seed);
  
  double md,kd,nd;
  size_t m, k, n;
  md=kd=nd=1.0;
  m=k=n=1;
  
  float increment = pow(max_footprint /sizeof(double),0.5)/ points1d;//Linear sampling
  float startX = increment+1;
  float startY = increment+1;
  float startZ = increment+1;

  //md: random range; m: random
  for (md=startX, m=(md); mem_footprint((md),startY,startZ) < max_footprint; md+=increment, m=(md)) {
    for (kd=startY, k=(kd); mem_footprint((md),(kd),startZ) < max_footprint; kd+=increment, k=(kd)) {
      for (nd=startZ, n=(nd); mem_footprint((md),(kd),(nd)) < max_footprint; nd+=increment, n=(nd)) {
        m = (md-increment*(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
        n = (nd-increment*(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
        k = (kd-increment*(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
        if (mem_footprint(m,k,n) <= min_footprint || mem_footprint(m,k,n) >= max_footprint)
          continue;
        df.add_key({m,k,n});

      }
    }
  }


  // float increment = 1.0;//rexp2: 2 to the power of x and round
  // for (md=1.0, m=rexp2(md); mem_footprint(rexp2(md),1,1) < max_footprint; md+=increment, m=rexp2(md)) {
  //   for (kd=1.0, k=rexp2(kd); mem_footprint(rexp2(md),rexp2(kd),1) < max_footprint; kd+=increment, k=rexp2(kd)) {
  //     for (nd=1.0, n=rexp2(nd); mem_footprint(rexp2(md),rexp2(kd),rexp2(nd)) < max_footprint; nd+=increment, n=rexp2(nd)) {
  //       if (mem_footprint(m,k,n) <= min_footprint)
  //         continue;
  //       m = rexp2(md-(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
  //       n = rexp2(nd-(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
  //       k = rexp2(kd-(static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));
  //       df.add_key({m,k,n});
  //     }
  //   }
  // }

  df.output(out);//save domain file

  // // shuffle domain file
  // rapidcsv::Document csv_file(filename);//read domains
  // std::vector<std::string> cols = csv_file.GetColumn<std::string>(0);//get n_row
  // size_t n_rows = cols.size();

  // std::vector<std::vector<std::string>> row_vect;//stores each row
  // row_vect.reserve(n_rows);


  // for (int i = 0; i < n_rows; i++) {//need to avoid shuffle empty rows
  //   row_vect[i] = csv_file.GetRow<std::string>(i);
  // }

  // auto rng = std::default_random_engine {};
  // std::shuffle(std::begin(row_vect), std::end(row_vect), rng);

  // for(int i=0; i < n_rows; i++){
  //   csv_file.InsertRow<std::string>(i, row_vect[i]);
  // }

  // for(int i=0; i < n_rows; i++){
  //   csv_file.RemoveRow(i+n_rows);
  // }

  // csv_file.Save(filename);//write file

  return 0;
}