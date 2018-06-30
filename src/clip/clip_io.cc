#include "clip_io.h"

ClipIO::ClipIO() {

}

ClipIO::~ClipIO() {

}

std::vector<std::string> tokenize(const std::string &l) {
	std::vector<std::string> ret;
	std::string token;
	for (const char c : l) {
		if (c == 0x0A || c == 0x0D || c == ' ' || c == '\t') {
			if (token != "") {
				ret.push_back(token);
				token = "";
			}
			continue;
		}
		
		token += c;
	}

	if (token != "") {
		ret.push_back(token);
		token = "";
	}

	return ret;
}

bool parse_vertex(const std::vector<std::string> &tokens, Clip *clip) {
	if (tokens.size() != 4) {
		printf("Vert line has != 4 tokens: %d\n", (int) tokens.size());
		return false;
	}

	int index = atoi(tokens[1].c_str());
	if (index >= (int)clip->verts.size()) {
		clip->verts.resize(index + 1);
	}

	Vec2 pos(atof(tokens[2].c_str()), atof(tokens[3].c_str()));

	clip->verts[index].pos = pos;

	return true;
}

bool parse_poly(const std::vector<std::string> &tokens, Clip *clip) {
	if (tokens.size() < 2) {
		printf("Poly line has < 2 tokens: %d\n", (int)tokens.size());
		return false;
	}

	int index = atoi(tokens[1].c_str());
	if (index >= (int)clip->polys.size()) {
		clip->polys.resize(index + 1);
	}

	ClipPoly &cp = clip->polys[index];

	cp.resize(0);
	for (int i = 2; i < (int)tokens.size(); i++) {
		cp.push_back(atoi(tokens[i].c_str()));
	}

	return true;
}

static bool process_line(const std::string &l, Clip *clip) {
	if (l == "") {
		return true;
	}

	std::vector<std::string> tokens = tokenize(l);

	if (tokens.size() == 0) {
		return true;
	}

	const std::string &type = tokens[0];

	if (type == "vcount") {
		if (tokens.size() != 2) {
			printf("vcount line has !=2 tokens: %d\n", (int)tokens.size());
			return false;
		}

		int vcount = atoi(tokens[1].c_str());
		printf("Handling vcount: %d\n", vcount);
		clip->verts.resize(vcount);
	}

	if (type == "vert") {
		return parse_vertex(tokens, clip);
	}

	if (type == "pcount") {
		if (tokens.size() != 2) {
			printf("pcount line has !=2 tokens: %d\n", (int)tokens.size());
			return false;
		}

		int pcount = atoi(tokens[1].c_str());
		printf("Handling pcount: %d\n", pcount);
		clip->polys.resize(pcount);
	}

	if (type == "poly") {
		return parse_poly(tokens, clip);
	}

	return true;
}

bool ClipIO::load(const char *filename, Clip *clip) {
	clip->verts.resize(0);
	clip->polys.resize(0);

	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Failed to open %s for reading\n", filename);
		return false;
	}

	fseek(fp, 0, SEEK_END);
	int sz = (int)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::string line;
	for (int i = 0; i < sz; i++) {
		char c = getc(fp);
		if (c == 0x0A || c == 0x0D) {
			if (!process_line(line, clip)) {
				fclose(fp);
				return false;
			}
			line = "";
			continue;
		}

		line += c;
	}

	if (line != "") {
		if (!process_line(line, clip)) {
			fclose(fp);
			return false;
		}
	}

	fclose(fp);

	for (ClipPoly &cp : clip->polys) {
		cp.cache(*clip);
	}

	return true;
}

bool ClipIO::save(const char *filename, const Clip &clip) {
	FILE *fp = fopen(filename, "w");
	if (!fp) {
		printf("Failed to open file for writing: %s\n", filename);
		return false;
	}

	// write vertices
	fprintf(fp, "vcount %d\n", (int)clip.verts.size());
	for (int i = 0; i < (int)clip.verts.size(); i++) {
		const ClipVertex &cv = clip.verts[i];
		fprintf(fp, "vert %d %.02f %.02f\n", i, cv.pos.x, cv.pos.y);
	}

	fprintf(fp, "\n");

	// write polys
	fprintf(fp, "pcount %d\n", (int)clip.polys.size());
	for (int i = 0; i < (int)clip.polys.size(); i++) {
		const ClipPoly &cp = clip.polys[i];
		fprintf(fp, "poly %d ", i);
		for (const int v : cp) {
			fprintf(fp, "%d ", v);
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "\n");

	fclose(fp);

	return true;
}