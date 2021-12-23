
const float ant_home_x = window_start_width / 2;
const float ant_home_y = window_start_height / 2;
const int memory_constant = 2000; //max distance from last target = memory_frames * velocity
int home_food_count = 0;

// Ants constants
const float sight_angle = PI * 0.25f, sight_radius = 30* common_constant, reach_radius = 10* common_constant, pheromone_radius = 40* common_constant, ant_velocity = 0.1f* common_constant;
const int number_of_ants = 40, pheromones_per_ant = 15, pheromone_max_frames = 2000; // have like constant in name

class ants
{
public:
	float x;
	float y;
	float rotation;
	int food_seen_id;
	int memory_frames;
	bool seen_target;
	bool centered_target;
	bool carrying_food;

	void start_values() {
		x = ant_home_x;
		y = ant_home_y;
		rotation = rand() % 100 * PI / 50;
		memory_frames = memory_constant;
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
		food_count = 30;
	}
};

struct pheromones
{
	float x;
	float y;
	bool from_home;

	void create(float ant_x, float ant_y, bool carrying_food) {
		x = ant_x;
		y = ant_y;
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
int pheromone_timer = pheromone_max_frames / pheromones_per_ant; 


static void
simulation() {
	float distance = 0;
	float angle = 0;
	float tactile_angle;
	float pheromone_angle;
	// Creating pheromones
	if (pheromone_timer == 0) {
		for (int n = 0; n < number_of_ants; n++) {
			pheromone[n * pheromones_per_ant + current_pheromone].create(ant[n].x, ant[n].y, ant[n].carrying_food);
		}
		current_pheromone++;
	}
	// fix that they follow wrong or all trails mabye cause pheromone_angle is shared

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
					if (m != n && ant[m].carrying_food != ant[n].carrying_food && get_distance(ant[n].x, ant[n].y, ant[m].x, ant[m].y) <= reach_radius && ant[m].memory_frames > 0) {
						tactile_angle = get_angle(get_delta_x(ant[n].x, food[ant[m].food_seen_id].x), get_delta_y(ant[n].y, food[ant[m].food_seen_id].y), distance, ant[n].rotation);
					}
				}
				// Pheromone sense
				for (int m = 0; m < number_of_ants * pheromones_per_ant; m++) { // dont always check
					if (!pheromone[m].from_home) {
						distance = get_distance(ant[n].x, ant[n].y, pheromone[m].x, pheromone[m].y); // Swallows up 70 % maybe square check first then that why does it do it in start
						if (distance <= pheromone_radius) {
							float sense_angle = get_angle(get_delta_x(ant[n].x, pheromone[m].x), get_delta_y(ant[n].y, pheromone[m].y), distance, ant[n].rotation);
							if (sense_angle <= abs(pheromone_sense_angle)) {
								pheromone_angle = pheromone_function(sense_angle);
							}
						}
					}
				}
			} // Pick up food
			else {
				if (food[ant[n].food_seen_id].food_count > 0 && get_distance(ant[n].x, ant[n].y, food[ant[n].food_seen_id].x, food[ant[n].food_seen_id].y) <= reach_radius) {
					food[ant[n].food_seen_id].food_count--;
					ant[n].carrying_food = true;
					ant[n].seen_target = false;
					ant[n].centered_target = false;
					ant[n].memory_frames = memory_constant;
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
					if (m != n && ant[m].carrying_food != ant[n].carrying_food && get_distance(ant[n].x, ant[n].y, ant[m].x, ant[m].y) <= reach_radius && ant[m].memory_frames > 0) {
						tactile_angle = get_angle(get_delta_x(ant[n].x, ant_home_x), get_delta_y(ant[n].y, ant_home_y), distance, ant[n].rotation);
					}
				}
				// Pheromone sense
				for (int m = 0; m < number_of_ants * pheromones_per_ant; m++) {
					if (pheromone[m].from_home) {
						distance = get_distance(ant[n].x, ant[n].y, pheromone[m].x, pheromone[m].y);
						if (distance <= pheromone_radius) {
							float sense_angle = get_angle(get_delta_x(ant[n].x, pheromone[m].x), get_delta_y(ant[n].y, pheromone[m].y), distance, ant[n].rotation);
							if (sense_angle <= abs(pheromone_sense_angle)) {
								pheromone_angle = pheromone_function(sense_angle);
							}
						}
					}
				}
			}
			// drop off food
			else {
				if (get_distance(ant[n].x, ant[n].y, ant_home_x, ant_home_y) <= reach_radius) {
					home_food_count++;
					ant[n].carrying_food = false;
					ant[n].seen_target = false;
					ant[n].centered_target = false;
					ant[n].memory_frames = memory_constant;
				}
			}
		}
		// Reducing ants memory
		if (ant[n].memory_frames > 0) {
			ant[n].memory_frames--; // could have one for all also iterations instead of frames
		}
		
		// Rotation and movement
		ant[n].set_rotation(angle, tactile_angle, pheromone_angle);
		ant[n].collision();
		ant[n].movement();
	}

	// changing pheromones variables that depend on frames
	pheromone_timer--;
	if (pheromone_timer < 0) {
		pheromone_timer = pheromone_max_frames / pheromones_per_ant;
	}
	if (current_pheromone >= pheromones_per_ant) {
		current_pheromone = 0;
	}

	// Render
	clear_screen(0x051019);
	for (int n = 0; n < number_of_ants * pheromones_per_ant; n++) {
		if (pheromone[n].from_home) {
			draw_rect(pheromone[n].x, pheromone[n].y, 1* common_constant, 1* common_constant, 0x260dae);
		}
		else {
			draw_rect(pheromone[n].x, pheromone[n].y, 1* common_constant, 1* common_constant, 0x10ff10);
		}
	}
	for (int n = 0; n < number_of_ants; n++) {
		draw_rect(ant[n].x, ant[n].y, 4* common_constant, 4* common_constant, 0xff0000);
	}
	for (int n = 0; n < number_of_foods; n++) {
		if (food[n].food_count > 0) {
			draw_rect(food[n].x, food[n].y, 10* common_constant, 10* common_constant, 0x10ff10);
		}
	}
	draw_rect(ant_home_x, ant_home_y, 10* common_constant, 10* common_constant, 0x260dae);
	}


