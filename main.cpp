extern "C" {
#include "tree.h"
};

#include "test.hpp"

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	int status = RUN_ALL_TESTS();
	fclose(log_stream);
	return status;
}
