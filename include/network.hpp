#include "matrix.hpp"
#include <vector>
#include <random>
#include <ctime>

using std::vector;
using std::string;
using std::time;

// quant. de neurônios no input, nas intermediárias e no output
template<int x, int y, int z, int layers_size> // input_width, layer_width, output_width
class network{
	//int &size_input, &size_inter, &size_output;

	static std::default_random_engine rand_gen;
	static std::uniform_real_distribution<float> dist;

	int fitness;

	matrix<x, 1> input;
	matrix<y, x> weights_input;

	vector<matrix<y, 1>> neurons;
	vector<matrix<y, y>> weights;

	matrix<z, y> weights_output;
	matrix<z, 1> output;

	  public:
 	network() : input(), weights_input(),
	neurons(layers_size, matrix<y, 1>()), weights(layers_size, matrix<y, y>()),
 	weights_output(), output()
	{
		if(layers_size != 1)
			weights = vector<matrix<y, y>>(max(layers_size - 1, 0));

		
		for(int i = 0; i < x; i++)
			for(int j = 0; j < y; j++)
				weights_input[j][i] = dist(rand_gen);

		for(int k = 0; k < layers_size; k++){
			weights.push_back(matrix<y, y>());

			for(int j = 0; j < y; j++)
				for(int i = 0; i < y; i++)
		 			weights.at(k)[j][i] = dist(rand_gen);
		}


		for(int i = 0; i < z; i++)
			for(int j = 0; j < y; j++)
				weights_output[i][j] = dist(rand_gen);
	}

	void calc(matrix<x, 1> input_values)
	{
		input = input_values;

		neurons[0] = weights_input * input;
		neurons[0].ReLu();

		for(int k = 1; k < layers_size; k++)
		{
			neurons[k] = weights[k-1] * neurons[k-1];
			neurons[k].ReLu();
		}

		output = weights_output * neurons[layers_size - 1];
		output.boolean();

		fitness++;
	}

	matrix<y, 1> getMatrixOutput()
	{
		return output;
	}
	
	array<float, z> getOutput()
	{
		return output.transpose()[0];
	}

	string print()
	{
		string str = weights_input.print();
		for(auto &p : weights)
			str.append(p.print());
		str.append(weights_output.print());
		return str;
	}

	int getFitness()
	{
		return fitness;
	}
};




template<int x, int y, int z, int layers_size>
std::default_random_engine network<x, y, z, layers_size>::rand_gen(time(0));
template<int x, int y, int z, int layers_size>
std::uniform_real_distribution<float> network<x, y, z, layers_size>::dist(-1000, 1000);