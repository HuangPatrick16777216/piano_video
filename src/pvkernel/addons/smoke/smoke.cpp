//
//  Piano Video
//  A free piano visualizer.
//  Copyright Patrick Huang 2021
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#define  SIZE_INT  sizeof(int)
#define  SIZE_FLT  sizeof(float)

#define  AIR_RESIST  0.95
#define  MAX_AGE     6

#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include "../../random.hpp"
#include "../../utils.hpp"


struct Particle {
    Particle() {
        good = true;
    }

    bool good;  // Whether to read from cache

    float age;  // Seconds

    // x, y are pixel locations.
    // vx, vy are pixel per frame values.
    float x, y, vx, vy;
};


void read_cache(std::vector<Particle>& ptcls, std::ifstream& fp) {
    if (!fp.good()) {
        std::cerr << "WARNING: smoke.cpp, read_cache: Cannot read file." << std::endl;
        return;
    }
    int count;
    fp.read((char*)(&count), SIZE_INT);

    for (int i = 0; i < count; i++) {
        Particle ptcl;
        fp.read((char*)(&ptcl), sizeof(Particle));
        if (ptcl.good)
            ptcls.push_back(ptcl);
    }
}

void write_cache(std::vector<Particle>& ptcls, std::ofstream& fp) {
    if (!fp.good()) {
        std::cerr << "WARNING: smoke.cpp, write_cache: Cannot write file." << std::endl;
        return;
    }

    const int size = ptcls.size();

    fp.write((char*)(&size), SIZE_INT);
    for (int i = 0; i < size; i++) {
        const Particle& ptcl = ptcls[i];
        fp.write((char*)(&ptcl), sizeof(Particle));
    }
}


extern "C" void smoke_sim(CD fps, const int num_new, const int num_notes, CD* const x_starts,
        CD* const x_ends, CD y_start, CD x_vel_min, CD x_vel_max, CD y_vel_min, CD y_vel_max,
        const char* const ip, const char* const op, const int width, const int height) {
    /*
    Simulate one frame of smoke activity.

    :param fps: Video fps.
    :param num_new: Number of new particles to generate.
    :param num_notes: Number of notes that are playing.
    :param x_starts, x_ends: X coordinate boundaries for each note.
    :param y_start: Y coordinate.
    :param x_vel_min, x_vel_max, y_vel_min, y_vel_max: Pixels per second bounds.
    :param ip: Input file path (leave blank if no input).
    :param op: Output file path.
    */

    CD vx_min = x_vel_min/fps, vx_max = x_vel_max/fps;
    CD vy_min = y_vel_min/fps, vy_max = y_vel_max/fps;

    std::vector<Particle> ptcls;
    ptcls.reserve((int)1e6);

    // Read from input file
    if (strlen(ip) > 0) {
        std::ifstream fin(ip);
        read_cache(ptcls, fin);
    }

    // Add new particles
    for (int i = 0; i < num_notes; i++) {
        for (int j = 0; j < num_new; j++) {
            Particle ptcl;
            ptcl.x = Random::uniform(x_starts[i], x_ends[i]);
            ptcl.y = y_start;
            ptcl.vx = Random::uniform(vx_min, vx_max);
            ptcl.vy = Random::uniform(vy_min, vy_max);
            ptcls.push_back(ptcl);
        }
    }

    const int size = ptcls.size();
    CD air_resist = std::pow(AIR_RESIST, 1/fps);

    // Simulate motion
    for (int i = 0; i < size; i++) {
        Particle& ptcl = ptcls[i];
        ptcl.x += ptcl.vx;
        ptcl.y += ptcl.vy;
        if (!img_bounds(width, height, ptcl.x, ptcl.y)) {
            ptcl.good = false;
            continue;
        }
        if (ptcl.age > MAX_AGE) {
            ptcl.good = false;
            continue;
        }
        ptcl.vx *= air_resist;
        ptcl.vy *= air_resist;
        ptcl.age += 1/fps;
    }

    // Write to output
    std::ofstream fout(op);
    write_cache(ptcls, fout);
}


extern "C" void smoke_render(UCH* img, const int width, const int height,
        const char* const path, CD intensity) {
    /*
    Render smoke on the image.

    :param path: Input cache path.
    :param intensity: Intensity multiplier.
    */

    std::ifstream fp(path);
    std::vector<Particle> ptcls;
    ptcls.reserve((int)1e6);
    read_cache(ptcls, fp);

    const int size = ptcls.size();

    for (int i = 0; i < size; i++) {
        const int x = (int)ptcls[i].x, y = (int)ptcls[i].y;

        if (img_bounds(width, height, x, y)) {
            const UCH value = 255 * (1-(ptcls[i].age/MAX_AGE));
            const UCH white[3] = {value, value, value};

            UCH original[3], modified[3];
            img_getc(img, width, x, y, original);
            img_mix(modified, original, white, intensity/10.0);
            img_setc(img, width, x, y, modified);

            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    const int nx = x+dx, ny = y+dy;
                    if (img_bounds(width, height, nx, ny)) {
                        UCH original[3], modified[3];
                        img_getc(img, width, nx, ny, original);
                        img_mix(modified, original, white, intensity/30.0);
                        img_setc(img, width, nx, ny, modified);
                    }
                }
            }
        }
    }
}