#include "assembler.h"

int main(void) {
    AssemblerPaths paths = {
        DEFAULT_OPTAB_PATH,
        DEFAULT_SRC_PATH,
        DEFAULT_INT_PATH,
        DEFAULT_OBJ_PATH,
        DEFAULT_OUTPUT_DIR
    };

    /* TODO: accept paths from argv while keeping these defaults for examples. */
    ensure_directory(paths.output_dir);

    load_optab(paths.optab_path);

    AssemblerContext ctx;
    pass1(&ctx, &paths);
    print_optab_dump();
    print_symtab_dump();
    print_littab_dump();
    pass2(&ctx, &paths);
    return 0;
}
