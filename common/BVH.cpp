
#include "BVH.h"

#include <cmath>
#include <cstring>

#include <fstream>

using namespace bvh;

BVH::BVH() {
    motion = nullptr;
    Clear();
}

BVH::BVH(const char *bvh_file_name) {
    motion = nullptr;
    Clear();

    Load(bvh_file_name);
}

BVH::~BVH() {
    Clear();
}

void BVH::Clear() {
    int i;
    for (i = 0; i < channels.size(); i++)
        delete channels[i];
    for (i = 0; i < joints.size(); i++)
        delete joints[i];
    if (motion != nullptr)
        delete motion;

    is_load_success = false;

    file_name = "";
    motion_name = "";

    num_channel = 0;
    channels.clear();
    joints.clear();
    joint_index.clear();

    num_frame = 0;
    interval = 0.0;
    motion = nullptr;
}

void BVH::Load(const char *bvh_file_name) {
#define BUFFER_LENGTH (1024 * 32)

    ifstream file;
    char line[BUFFER_LENGTH];
    char *token;
    char separater[] = " :,\t";
    vector<Joint *> joint_stack;
    Joint *joint = nullptr;
    Joint *new_joint = nullptr;
    bool is_site = false;
    double x, y, z;
    int i, j;

    Clear();

    // assign motion name
    file_name = bvh_file_name;
    const char *mn_first = bvh_file_name;
    const char *mn_last = bvh_file_name + strlen(bvh_file_name);
    if (strrchr(bvh_file_name, '\\') != nullptr)
        mn_first = strrchr(bvh_file_name, '\\') + 1;
    else if (strrchr(bvh_file_name, '/') != nullptr)
        mn_first = strrchr(bvh_file_name, '/') + 1;
    if (strrchr(bvh_file_name, '.') != nullptr)
        mn_last = strrchr(bvh_file_name, '.');
    if (mn_last < mn_first)
        mn_last = bvh_file_name + strlen(bvh_file_name);
    motion_name.assign(mn_first, mn_last);

    file.open(bvh_file_name, ios::in);
    if (file.is_open() == 0) return;

    while (!file.eof()) {
        if (file.eof())
            goto bvh_error;

        file.getline(line, BUFFER_LENGTH);
        token = strtok(line, separater);

        if (token == nullptr) continue;

        if (strcmp(token, "{") == 0) {
            joint_stack.push_back(joint);
            joint = new_joint;
            continue;
        }
        if (strcmp(token, "}") == 0) {
            joint = joint_stack.back();
            joint_stack.pop_back();
            is_site = false;
            continue;
        }

        if ((strcmp(token, "ROOT") == 0) ||
            (strcmp(token, "JOINT") == 0)) {
            new_joint = new Joint();
            new_joint->index = joints.size();
            if (joint != nullptr) {
                new_joint->parents.assign(joint->parents.begin(), joint->parents.end());
                new_joint->parents.push_back(joint);
            }
            new_joint->has_site = false;
            new_joint->offset[0] = 0.0;
            new_joint->offset[1] = 0.0;
            new_joint->offset[2] = 0.0;
            new_joint->site[0] = 0.0;
            new_joint->site[1] = 0.0;
            new_joint->site[2] = 0.0;
            joints.push_back(new_joint);
            if (joint)
                joint->children.push_back(new_joint);

            token = strtok(nullptr, "");
            while (*token == ' ') token++;
            new_joint->name = token;

            joint_index[new_joint->name] = new_joint;
            continue;
        }

        if ((strcmp(token, "End") == 0)) {
            new_joint = joint;
            is_site = true;
            continue;
        }

        if (strcmp(token, "OFFSET") == 0) {
            token = strtok(nullptr, separater);
            x = token ? atof(token) : 0.0;
            token = strtok(nullptr, separater);
            y = token ? atof(token) : 0.0;
            token = strtok(nullptr, separater);
            z = token ? atof(token) : 0.0;

            if (is_site) {
                joint->has_site = true;
                joint->site[0] = x;
                joint->site[1] = y;
                joint->site[2] = z;
            } else {
                joint->offset[0] = x;
                joint->offset[1] = y;
                joint->offset[2] = z;
            }
            continue;
        }

        if (strcmp(token, "CHANNELS") == 0) {
            token = strtok(nullptr, separater);
            joint->channels.resize(token ? atoi(token) : 0);

            rotationOrder.push_back(vector<ChannelEnum>());
            for (i = 0; i < joint->channels.size(); i++) {
                Channel *channel = new Channel();
                channel->joint = joint;
                channel->index = channels.size();
                channels.push_back(channel);
                joint->channels[i] = channel;

                token = strtok(nullptr, separater);
                if (strcmp(token, "Xrotation") == 0) {
                    channel->type = X_ROTATION;
                    rotationOrder.back().push_back(channel->type);
                } else if (strcmp(token, "Yrotation") == 0) {
                    channel->type = Y_ROTATION;
                    rotationOrder.back().push_back(channel->type);
                } else if (strcmp(token, "Zrotation") == 0) {
                    channel->type = Z_ROTATION;
                    rotationOrder.back().push_back(channel->type);
                } else if (strcmp(token, "Xposition") == 0)
                    channel->type = X_POSITION;
                else if (strcmp(token, "Yposition") == 0)
                    channel->type = Y_POSITION;
                else if (strcmp(token, "Zposition") == 0)
                    channel->type = Z_POSITION;
            }
        }

        if (strcmp(token, "MOTION") == 0)
            break;
    }

    file.getline(line, BUFFER_LENGTH);
    token = strtok(line, separater);
    if (strcmp(token, "Frames") != 0)
        goto bvh_error;
    token = strtok(nullptr, separater);
    if (token == nullptr)
        goto bvh_error;
    num_frame = atoi(token);

    file.getline(line, BUFFER_LENGTH);
    token = strtok(line, ":");
    if (strcmp(token, "Frame Time") != 0)
        goto bvh_error;
    token = strtok(nullptr, separater);
    if (token == nullptr)
        goto bvh_error;
    interval = atof(token);

    num_channel = channels.size();
    motion = new double[num_frame * num_channel];

    for (i = 0; i < num_frame; i++) {
        file.getline(line, BUFFER_LENGTH);
        token = strtok(line, separater);
        for (j = 0; j < num_channel; j++) {
            if (token == nullptr)
                goto bvh_error;
            motion[i * num_channel + j] = atof(token);
            token = strtok(nullptr, separater);
        }
    }

    file.close();

    is_load_success = true;

    return;

    bvh_error:
    file.close();
}
