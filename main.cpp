#include <stdio.h>
#include "include/game.hpp"

int main()
{
	Font font;
	font.loadFromFile("fonts\\Roboto-Medium.ttf");

	Text status_population("", font, 26.25);
	{
		status_population.setFillColor(Color::White);
		status_population.setOutlineColor(Color::Black);
		status_population.setOutlineThickness(3.5);
	}

	Texture tex;
	auto [rand_gen, background, terrain, rand_dist, tubes] = InitGame(tex);

	RenderWindow window(VideoMode(378, 819), "sfml", Style::Titlebar | Style::Close);
	window.setFramerateLimit(60);

	auto [birds, IAs] = InitBirds(tex, window);

	while (window.isOpen())
	{
		Event e;
		while (window.pollEvent(e))
		{
			switch (e.type)
			{
			case Event::Closed:
				CloseGame(window, IAs);
				window.close();
				break;
			}
		}

		for (int i = 0; i < population_size; i++)
			for (auto &g : terrain)
				if (birds.at(i).checkColision(g.getGlobalBounds()) && !gameover.at(i))
				{
					gameover_birds++;
					if (!dead.at(i))
						dead_birds++;
					dead.at(i) = true;
					gameover.at(i) = true;
					if (dead_birds == population_size)
						speed = 0;
				}

		for (int i = 0; i < population_size; i++)
			for (auto &b : tubes.at(index_tube % 3).getSprites())
				if (birds.at(i).checkColision(b.getGlobalBounds()) && !dead.at(i))
				{
					dead_birds++;
					birds.at(i).kill();
					dead.at(i) = true;
					if (dead_birds == population_size)
						speed = 0;
				}

		CheckCurrentBirdState(tubes, birds);

		if (gameover_birds < population_size)
		{
			BirdsUpdate(IAs, tubes, birds, window);
			MoveScenary(terrain, tubes, tex, window, rand_dist, rand_gen);
		}
		else
		{
			static Clock clock;
			if (clock.getElapsedTime() >= seconds(3))
			{
				CloseGame(window, IAs);
				window.close();
			}
		}

		status_population.setString("Tube: " + to_string(index_tube) +
									"\nPopulation: " + to_string(population_size) +
									"\nDeads: " + to_string(dead_birds) +
									"\nLive: " + to_string(population_size - dead_birds) +
									"\nCurrent Bird: " + to_string(current_bird) + "(" + to_string(int(getNextTube(tubes, birds[current_bird])[0])) + ", " + to_string(int(getNextTube(tubes, birds[current_bird])[1])) + ")");

		Draw(window, background, tubes, birds, terrain, status_population);
	}
}
