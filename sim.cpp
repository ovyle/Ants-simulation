// Constants
const float ant_home_x = window_start_width / 2;
const float ant_home_y = window_start_height / 2;
const int memory_constant = 2000; //max distance from last target = memory_iterations * velocity
const int pheromone_max_strenght = 10000;
const int food_count_start = 100;
const int number_of_ants = 100; 
const int pheromones_per_ant = 15;
const int pheromone_max_frames = 2000;
const float sight_angle = PI * 0.25f;
const float ant_velocity = 0.1f;

// Constants changed between simulation settings 
const float sight_radius = 40, scent_radius = 40, reach_radius = 10;

// Variables
int home_food_count = 0;
bool data_save = false;
int iteration_count = 0;
int simulation_count = 0;


class ants
{
public:
	float x;
	float y;
	float rotation;
	int food_seen_id;
	int memory_iterations;
	int pheromone_strenght;
	bool seen_target;
	bool centered_target;
	bool carrying_food;

	void start_values() {
		x = ant_home_x;
		y = ant_home_y;
		rotation = rand() % 100 * PI / 50;
		memory_iterations = memory_constant;
		pheromone_strenght = pheromone_max_strenght;
		seen_target = false;
		centered_target = false;
		carrying_food = false;
	}
	void set_rotation(float angle, float tactile_angle, float pheromone_angle) {
		if (seen_target) {
			if (!centered_target) {
				rotation += angle;
				centered_target = true;
			}
		}
		else {
			rotation += positive_negative * (rand() % 10 * 0.003f) + tactile_angle + pheromone_angle;
		}
	}
	void movement() {
		x = x + ant_velocity * cosf(rotation);
		y = y - ant_velocity * sinf(rotation);
	}
	void collision() {
		if (x >= window_start_width) {
			if (rotation <= PI / 2) {
				rotation = PI - rotation;
			}
			else {
				rotation = 2 * PI - rotation;
			}
		}
		else if (x <= 0) {
			if (rotation <= PI)
			{
				rotation = PI - rotation;
			}
			else {
				rotation = 3 * PI - rotation;
			}
		}
		else if (y >= window_start_height || y <= 0) {
			rotation = 2 * PI - rotation;
		}
	}
};

const int number_of_foods = 3;

struct foods
{
	float x;
	float y;
	int r; //distance form ant home
	int food_count;

	void start_values(int n) {
		r = ant_home_y / number_of_foods * (n + 1) - 10;
		x = ant_home_x + rand() % (2 * r + 1) - r; // cirkel equation
		y = ant_home_y + positive_negative * sqrtf(-powf(ant_home_x, 2) + 2 * ant_home_x * x + powf(r, 2) - powf(x, 2));
		food_count = food_count_start;
	}
};

struct pheromones
{
	float x;
	float y;
	int strenght;
	bool from_home;


	void create2(float ant_x, float ant_y, int pheromone_strength, bool carrying_food) {
		x = ant_x;
		y = ant_y;
		strenght = pheromone_strength;
		if (!carrying_food) {
			from_home = true;
		}
		else {
			from_home = false;
		}
	}
};

ants ant[number_of_ants];
foods food[number_of_foods];
pheromones pheromone[number_of_ants*pheromones_per_ant];

static void
simulation_startup() {
	for (int n = 0; n < number_of_foods; n++) {
		food[n].start_values(n);
	}
	for (int n = 0; n < number_of_ants; n++) {
		ant[n].start_values();
	}
}

int current_pheromone = 0;
int pheromone_create_timer = pheromone_max_frames / pheromones_per_ant; 
int pheromone_check_timer_constant = 5;
int pheromone_check_timer = pheromone_check_timer_constant;

static void
simulation() {
	iteration_count++;

	float distance = 0;
	float angle = 0;
	float tactile_angle;
	float pheromone_angle;

	// Creating pheromones
	if (pheromone_create_timer == 0) {
		for (int n = 0; n < number_of_ants; n++) {
			pheromone[n * pheromones_per_ant + current_pheromone].create2(ant[n].x, ant[n].y, ant[n].pheromone_strenght, ant[n].carrying_food);
		}
		current_pheromone++;
	}

	// Ants behavior
	for (int n = 0; n < number_of_ants; n++) {
		tactile_angle = 0;
		float pheromone_angle = 0;
		// Goal = Food 
		if (!ant[n].carrying_food) {
			if (!ant[n].seen_target) {
				// Sight sense
				for (int m = 0; m < number_of_foods; m++) {
					if (food[m].food_count > 0) {
						distance = get_distance(ant[n].x, ant[n].y, food[m].x, food[m].y);
						if (distance <= sight_radius) {
							angle = get_angle(get_delta_x(ant[n].x, food[m].x), get_delta_y(ant[n].y, food[m].y), distance, ant[n].rotation);
							if (abs(angle) <= sight_angle) {
								ant[n].seen_target = true;
								ant[n].food_seen_id = m;
								break;
							}
						}
					}
				} 
				// Tactile signals
				for (int m = 0; m < number_of_ants; m++)
				{
					if (m != n && ant[m].carrying_food != ant[n].carrying_food && get_distance(ant[n].x, ant[n].y, ant[m].x, ant[m].y) <= reach_radius && ant[m].memory_iterations > 0) {
						tactile_angle = get_angle(get_delta_x(ant[n].x, food[ant[m].food_seen_id].x), get_delta_y(ant[n].y, food[ant[m].food_seen_id].y), distance, ant[n].rotation);
					}
				}
				// Pheromone sense
				if (pheromone_check_timer == 0) {
					for (int m = 0; m < number_of_ants * pheromones_per_ant; m++) { // dont always check
						if (!pheromone[m].from_home) {
							distance = get_distance(ant[n].x, ant[n].y, pheromone[m].x, pheromone[m].y); // Swallows up 70 % maybe square check first then that why does it do it in start
							if (distance <= scent_radius) {
								float sense_angle = get_angle(get_delta_x(ant[n].x, pheromone[m].x), get_delta_y(ant[n].y, pheromone[m].y), distance, ant[n].rotation);
								if (abs(sense_angle) <= pheromone_sense_angle) {
									pheromone_locate(sense_angle, pheromone[m].strenght);
								}
							}
						}
					}
					pheromone_angle = pheromone_direction();
					pheromone_concentration_reset();
				}
			} // Pick up food
			else {
				if (food[ant[n].food_seen_id].food_count > 0 && get_distance(ant[n].x, ant[n].y, food[ant[n].food_seen_id].x, food[ant[n].food_seen_id].y) <= reach_radius) {
					food[ant[n].food_seen_id].food_count--;
					ant[n].carrying_food = true;
					ant[n].seen_target = false;
					ant[n].centered_target = false;
					ant[n].memory_iterations = memory_constant;
					ant[n].pheromone_strenght = pheromone_max_strenght;
					ant[n].rotation += PI;
				}
			}
		} 
		// Goal = Home
		else {
			if (!ant[n].seen_target) {
				// Sight sense
				distance = get_distance(ant[n].x, ant[n].y, ant_home_x, ant_home_y);
				if (distance <= sight_radius) {
					angle = get_angle(get_delta_x(ant[n].x, ant_home_x), get_delta_y(ant[n].y, ant_home_y), distance, ant[n].rotation);
					if (abs(angle) <= sight_angle) {
						ant[n].seen_target = true;
					}
				} 
				//tactile signals
				for (int m = 0; m < number_of_ants; m++)
				{
					if (m != n && ant[m].carrying_food != ant[n].carrying_food && get_distance(ant[n].x, ant[n].y, ant[m].x, ant[m].y) <= reach_radius && ant[m].memory_iterations > 0) {
						tactile_angle = get_angle(get_delta_x(ant[n].x, ant_home_x), get_delta_y(ant[n].y, ant_home_y), distance, ant[n].rotation);
					}
				}
				// Pheromone sense
				if (pheromone_check_timer == 0) {
					for (int m = 0; m < number_of_ants * pheromones_per_ant; m++) {
						if (pheromone[m].from_home) {
							distance = get_distance(ant[n].x, ant[n].y, pheromone[m].x, pheromone[m].y);
							if (distance <= scent_radius) {
								float sense_angle = get_angle(get_delta_x(ant[n].x, pheromone[m].x), get_delta_y(ant[n].y, pheromone[m].y), distance, ant[n].rotation);
								if (abs(sense_angle) <= pheromone_sense_angle) {
									pheromone_locate(sense_angle, pheromone[m].strenght);
								}
							}
						}
					}
					pheromone_angle = pheromone_direction();
					pheromone_concentration_reset();
				}
			}
			// drop off food
			else {
				if (get_distance(ant[n].x, ant[n].y, ant_home_x, ant_home_y) <= reach_radius) {
					home_food_count++;
					ant[n].carrying_food = false;
					ant[n].seen_target = false;
					ant[n].centered_target = false;
					ant[n].memory_iterations = memory_constant;
					ant[n].pheromone_strenght = pheromone_max_strenght;
					ant[n].rotation += PI;

					if (home_food_count == food_count_start* number_of_foods) {
						data_save = true;
					}
					if (home_food_count == 225) { // test food_count_start * number_of_foods*0.75
						data_save = true;
					}
				}
			}
		}
		// Reducing ants memory
		if (ant[n].memory_iterations > 0) {
			ant[n].memory_iterations--; // could have one for all also iterations instead of frames
		}
		if (ant[n].pheromone_strenght > 1500) {
			ant[n].pheromone_strenght--;
		}

		// Rotation and movement
		ant[n].set_rotation(angle, tactile_angle, pheromone_angle);
		ant[n].collision();
		ant[n].movement();
	}

	// changing pheromones variables that depend on iterations
	pheromone_create_timer--;
	pheromone_check_timer--;
	if (pheromone_create_timer < 0) {
		pheromone_create_timer = pheromone_max_frames / pheromones_per_ant;
	}
	if (pheromone_check_timer < 0) {
		pheromone_check_timer = pheromone_check_timer_constant;
	}
	if (current_pheromone >= pheromones_per_ant) {
		current_pheromone = 0;
	}

	// Render
	clear_screen(0x051019);
	for (int n = 0; n < number_of_ants * pheromones_per_ant; n++) {
		if (pheromone[n].from_home) {
			draw_rect(pheromone[n].x, pheromone[n].y, 1, 1, 0x260dae);
		}
		else {
			draw_rect(pheromone[n].x, pheromone[n].y, 1, 1, 0x10ff10);
		}
	}
	for (int n = 0; n < number_of_ants; n++) {
		draw_rect(ant[n].x, ant[n].y, 4, 4, 0xff0000);
	}
	for (int n = 0; n < number_of_foods; n++) {
		if (food[n].food_count > 0) {
			draw_rect(food[n].x, food[n].y, 10, 10, 0x10ff10);
		}
	}
	draw_rect(ant_home_x, ant_home_y, 10, 10, 0x260dae);
}

void variable_reset() {
	home_food_count = 0;
	current_pheromone = 0;
	for (int n = 0; n < number_of_ants * pheromones_per_ant; n++) {
		pheromone[n].from_home = true;
		pheromone[n].x = ant_home_x;
		pheromone[n].y = ant_home_y;
	}
}
