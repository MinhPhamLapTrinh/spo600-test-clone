/* Test pass
   Duc Minh Pham, dpham34@myseneca.ca, March 09 2025
   Modelled on tree-nrv.cc

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#define INCLUDE_MEMORY
#include "config.h"              /* One of the main headers that are loaded 
                     * into GCC source files. 
                     * Contains system-specific configuration details 
                     * determined during the build process */
#include "system.h"              /* Provides a standardized way to include 
                     * sys and std headers */
#include "coretypes.h"           /* This one contains fundamental type definitions and macros. */
#include "backend.h"             /* Main header for compiler's backend infrastructure. */
#include "tree.h"                /* Defines tree data structure, a GCC's primary internal representation
                   * of program structure. 
                   * This is one of our main tools for seeing and 
                   * using GCC representation and transformations of source code. */
#include "gimple.h"              /* Defines GIMPLE intermediate representations.
                     * We will use it with the previous header to prune our code later. */
#include "tree-pass.h"           /* A header that defines infrastructure for compiler passes. 
                        * Meaning, that this is our core header */
#include "ssa.h"                 /* A header that implements the Static Single Assignment (SSA) form. 
                  * It may contain something usefull for later iterations */
#include "gimple-iterator.h"     /* Iterators for GIMPLE statements. 
                              * Safe and convenient iteration over basic blocks and statements */
#include "gimple-walk.h"         /* Tools for traversing and analyzing GIMPLE representation.
                          * It might be useful in later iterations */
#include "internal-fn.h"         /* Defines compiler-specific functions? */
#include "gimple-pretty-print.h" /* Functions for printing GIMPLE representations in a human-readable format. */

// Added headers:
#include "gimple-ssa.h"
#include "cgraph.h"
#include "attribs.h"
#include "pretty-print.h"
#include "tree-inline.h"
#include "intl.h"
#include "basic-block.h"

#include <string>
#include <vector>
#include <map>
#include <sstream>

// =================================================== vvv
// Test passa
// Global storage for cloned functions.
// The key is the base function name, and the value points to the first clone
static std::map<std::string, char *> base_signatures;
static std::map<std::string, std::vector<char *>> clone_signatures;

// Global counter for total functions in the TU.
// Initialized once at the first call to execute.
static int remaining_funcs = -1;

static char *get_gimple_signature(function *fn)
{
    basic_block bb;
    int stmt_cnt = 0;

    // Start with an empty block (1 extra for the null terminator).
    char *codes = (char *)xmalloc((stmt_cnt + 1) * sizeof(char));

    FOR_EACH_BB_FN(bb, fn)
    {
        for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
        {
            // Increment the statement counter.
            stmt_cnt++;

            // Get the current GIMPLE statement.
            gimple *stmt = gsi_stmt(gsi);

            // Get the opcode (an integer value).
            int code = gimple_code(stmt);

            // Resize the array to hold one more character (+1 for null terminator).
            codes = (char *)xrealloc(codes, (stmt_cnt + 1) * sizeof(char));

            // Store the opcode as a character (shifted by 48 to make it printable).
            codes[stmt_cnt - 1] = code + 48;
        }
    }

    // Null-terminate the string.
    codes[stmt_cnt] = '\0';
    return codes;
}

static bool
compare_functions(char *sig1, char *sig2)
{
    bool same = (strcmp(sig1, sig2) == 0);

    if (dump_file)
    {
        fprintf(dump_file, "=== Signature for first function: ===\n%s\n", sig1);
        fprintf(dump_file, "=== Signature for second function: ===\n%s\n", sig2);
    }

    return same;
}
namespace
{

    const pass_data pass_data_dpham34 =
        {
            GIMPLE_PASS,   /* type */
            "dpham34",     /* name */
            OPTGROUP_NONE, /* optinfo_flags */
            TV_NONE,       /* tv_id */
            PROP_cfg,      /* properties_required */
            0,             /* properties_provided */
            0,             /* properties_destroyed */
            0,             /* todo_flags_start */
            0,             /* todo_flags_finish */
    };

    class pass_dpham34 : public gimple_opt_pass
    {
    public:
        /* Default constructor for our class */
        pass_dpham34(gcc::context *ctxt)
            : gimple_opt_pass(pass_data_dpham34, ctxt)
        {
        }

        /* opt_pass methods: */
        /* function that checks if this pass should be executed */
        /* by default returning 1 in gimple_opt_pass, but we set it to be sure */
        bool gate(function *) final override
        {
            return 1; // always execute pass
        }

        /* Function that will be called on each funciton of source code */
        unsigned int execute(function *fn) final override;

        void finished();
    }; // class pass_dpham34

    unsigned int
    pass_dpham34::execute(function *fn)
    {

        // Initialize remaining_funcs on the very first call.
        if (remaining_funcs < 0)
        {
            remaining_funcs = 0;
            struct cgraph_node *node;
            FOR_EACH_DEFINED_FUNCTION(node)
            {
                remaining_funcs++;
            }
            if (dump_file)
                fprintf(dump_file, "Total functions to process: %d\n", remaining_funcs);
        }

        if (dump_file)
        {
            fprintf(dump_file, "Function counter so far is: %d\n", remaining_funcs);
        }

        if (dump_file)
        {
            const char *fname = IDENTIFIER_POINTER(DECL_NAME(fn->decl));

            std::string s(fname);
            size_t pos = s.find(".");

            /* Compute the GIMPLE signature for this function */
            char *sig = get_gimple_signature(fn);

            if (pos == std::string::npos)
            {
                // Function name without a dot: treat it as the base function.
                base_signatures[s] = sig;
            }
            else
            {
                // Function name with a dot. Extract base and suffix.
                std::string base = s.substr(0, pos);
                std::string suffix = s.substr(pos + 1);

                // Skip resolver functions.
                if (suffix == "resolver")
                {
                    free(sig);
                    remaining_funcs--;
                    return 0;
                }

                // Record the clone.
                clone_signatures[base].push_back(sig);
                if (dump_file)
                {
                    fprintf(dump_file,
                            "=== Recorded cloned function: %s (base: %s) ===\n",
                            s.c_str(), base.c_str());
                }
            }
        }
        // Decrement our counter.
        remaining_funcs--;
        // If this is the last function processed, trigger finalization.
        if (remaining_funcs == 2)
            finished();
        return 0;
    }

    void pass_dpham34::finished()
    {
        if (dump_file)
        {
            fprintf(dump_file, "=== Finished processing functions ===\n");
            for (auto &entry : clone_signatures)
            {
                const std::string &base = entry.first;
                const std::vector<char *> &cloneSigs = entry.second;
                char *base_sig = nullptr;
                auto it = base_signatures.find(base);
                if (it != base_signatures.end())
                    base_sig = it->second;

                if (base_sig == nullptr)
                {
                    if (dump_file)
                        fprintf(dump_file, "No base signature found for cloned functions with base: %s\n", base.c_str());
                    continue;
                }

                bool all_match = true;
                for (char *clone_sig : cloneSigs)
                {
                    if (!compare_functions(base_sig, clone_sig))
                    {
                        all_match = false;
                        break;
                    }
                }

                if (dump_file)
                {
                    if (all_match)
                        fprintf(dump_file, "PRUNE: %s\n", base.c_str());
                    else
                        fprintf(dump_file, "NOPRUNE: %s\n", base.c_str());
                }
            }
            if (dump_file)
            {
                fprintf(dump_file,
                        "\n\n#### End dpham34 diagnostics, starting regular GIMPLE dump ####\n\n\n");
            }
        }

        /* Free all allocated signature strings and clear the maps */
        for (auto &b : base_signatures)
            free(b.second);
        base_signatures.clear();

        for (auto &c : clone_signatures)
        {
            for (char *s : c.second)
                free(s);
        }
        clone_signatures.clear();
    }
} // anon namespace

gimple_opt_pass *
make_pass_dpham34(gcc::context *ctxt)
{
    return new pass_dpham34(ctxt);
}

// =================================================== ^^^