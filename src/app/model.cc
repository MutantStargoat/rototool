#include "model.h"

Model::Model() {
	cur_video_frame = 0;
}

Model::~Model() {

}

int Model::get_cur_video_frame() const {
	return cur_video_frame;
}