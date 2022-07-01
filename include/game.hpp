#include <SFML/Graphics.hpp>
#include "network.hpp"
#include "bird.hpp"
#include "Tubes.hpp"
#include <fstream>
#include <string>

using namespace sf;

using std::array, std::minstd_rand, std::uniform_int_distribution, std::move, std::to_string;

const int population_size = 10000;
float speed = -1.35, scale;
int index_tube, dead_birds, gameover_birds, current_bird;
array<bool, population_size> dead, gameover;

typedef network<2, 2, 1, 1> NeuralNetwork;


auto InitGame(Texture &tex)
{
	minstd_rand rand_gen(time(0));

	tex.loadFromFile("..\\flappy-bird.png");
	Sprite background(tex, IntRect(0, 0, 144, 256));

	Vector2u window_s = Vector2u(378, 819);
	{
		auto [a, b, w_b, h_b] = background.getTextureRect();
		auto [w_w, h_w] = window_s;
		scale = (float(w_w / h_w) < float(w_b / h_b) ? float(w_w / w_b) : float(h_w / h_b));
		a--;
		b++;
	}

	background.scale(scale, scale);

	array<Sprite, 2> terrain{
		Sprite(tex, IntRect(146, 0, 154, 56)),
		Sprite(tex, IntRect(146, 0, 154, 56))};

	for (int i = 0; i < 2; i++)
	{
		terrain.at(i).scale(scale, scale);
		terrain.at(i).setPosition(terrain.at(i).getGlobalBounds().width * i, window_s.y - terrain.at(i).getGlobalBounds().height);
	}

	uniform_int_distribution<int> rand_dist(Tube::opening_size * 1.2, window_s.y - terrain.at(0).getGlobalBounds().height - Tube::opening_size * 1.2);

	array<Tube, 3> tubes{
		Tube(tex, scale, Vector2f(window_s.x * 1.5, rand_dist(rand_gen))),
		Tube(tex, scale, Vector2f(window_s.x * 2, rand_dist(rand_gen))),
		Tube(tex, scale, Vector2f(window_s.x * 2.5, rand_dist(rand_gen)))};
	return make_tuple(rand_gen, move(background), move(terrain), move(rand_dist), move(tubes));
}

auto InitBirds(Texture &tex, RenderWindow &window)
{

	vector<NeuralNetwork> IAs;
	vector<Bird> birds;

	IAs.reserve(population_size);
	for (int i = 0; i < population_size; i++)
		IAs.emplace_back();

	birds.reserve(population_size);
	for (int i = 0; i < population_size; i++)
		birds.emplace_back(tex, scale, Vector2f(window.getSize().x / 2, window.getSize().y / 2));

	return make_tuple(birds, IAs);
}

inline void MoveScenary(array<Sprite, 2> &terrain, array<Tube, 3> &tubes, Texture &tex, RenderWindow &window, uniform_int_distribution<int> &rand_dist, minstd_rand &rand_gen)
{

	for (auto &s : terrain)
	{
		s.move(speed, 0);
		if (s.getPosition().x < -s.getGlobalBounds().width)
			s.move(s.getGlobalBounds().width * 2, 0);
	}
	for (auto &s : tubes)
	{
		s.update(speed);
		if (s.getPosition().x < -s.getWidth())
			s = Tube(tex, scale, Vector2f(window.getSize().x * 1.5 - s.getWidth(), rand_dist(rand_gen)));
	}
}

array<float, 2> getNextTube(array<Tube, 3> &tubes, Bird &bird)
{
	return array<float, 2>{
		tubes.at(index_tube % 3).getPosition().x - bird.getPosition().x,
		tubes.at(index_tube % 3).getPosition().y - bird.getPosition().y};
};

inline void BirdsUpdate(vector<NeuralNetwork> &IAs, array<Tube, 3> &tubes, vector<Bird> &birds, RenderWindow &window)
{
	for (int i = 0; i < population_size; i++)
	{
		// update the birds
		if (!dead.at(i))
		{
			// Update the neural networks
			IAs.at(i).calc(matrix<1, 2>({getNextTube(tubes, birds.at(i))}).transpose());

			auto [decision] = IAs.at(i).getOutput();

			if (decision == 1.0f)
				birds.at(i).flap();

			birds.at(i).update();
		}
		else if (birds.at(i).getPosition().x >= -birds.at(i).getRadius()) // if the bird is not on the screen, he dont move
		{
			birds.at(i).deadUpdate(speed);
			if (!gameover.at(i))
				birds.at(i).update();
		}

		// upper flight limit
		if (birds.at(i).getPosition().y < birds.at(i).getRadius())
			birds.at(i).setPosition(window.getSize().x / 2, birds.at(i).getRadius());
	}
}

inline void CheckCurrentBirdState(array<Tube, 3> &tubes, vector<Bird> &birds)
{
	for (int temp_current_bird = 0; temp_current_bird < population_size; temp_current_bird++)
		if (!dead.at(temp_current_bird))
		{
			// update index tube
			if (tubes.at(index_tube % 3).getPosition().x + tubes.at(index_tube % 3).getWidth() / 2 <= birds.at(temp_current_bird).getPosition().x - birds.at(temp_current_bird).getRadius())
				if (tubes.at(index_tube % 3).getPosition().x + tubes.at(index_tube % 3).getWidth() / 2 - speed > birds.at(temp_current_bird).getPosition().x - birds.at(temp_current_bird).getRadius())
					index_tube++;

			current_bird = temp_current_bird;
			break;
		}
}

inline void Draw(RenderWindow &window, Sprite &background, array<Tube, 3> &tubes, vector<Bird> &birds, array<Sprite, 2> &terrain, Text &status_population)
{
	window.clear();
	window.draw(background);
	for (auto &tube : tubes)
		window.draw(tube);
	for (auto &b : birds) //(int i = 0; i < population_size; i++)
		if ((b.getPosition().x >= -b.getRadius()))
			window.draw(b);
	for (auto &ground : terrain)
		window.draw(ground);
	window.draw(status_population);
	window.display();
}

inline void CloseGame(RenderWindow &window, vector<NeuralNetwork> &IAs)
{

	std::string str = IAs.at(current_bird).print();

	std::fstream file;
	file.open("..\\saves\\save-1 t.txt", std::fstream::in | std::fstream::out | std::fstream::trunc);
	file.clear();
	file << str;
	file.close();

	Texture t;
	t.create(window.getSize().x, window.getSize().y);
	t.update(window);
	t.copyToImage().saveToFile("..\\screenshots\\img.png");
}
