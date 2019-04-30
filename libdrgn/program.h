// Copyright 2018-2019 - Omar Sandoval
// SPDX-License-Identifier: GPL-3.0+

/**
 * @file
 *
 * Program internals.
 *
 * See @ref ProgramInternals.
 */

#ifndef DRGN_PROGRAM_H
#define DRGN_PROGRAM_H

#include "memory_reader.h"
#include "symbol_index.h"
#include "type_index.h"

/**
 * @ingroup Internals
 *
 * @defgroup ProgramInternals Programs
 *
 * Program internals.
 *
 * @{
 */

/** The important parts of the VMCOREINFO note of a Linux kernel core. */
struct vmcoreinfo {
	/** <tt>uname -r</tt> */
	char osrelease[128];
	/**
	 * The offset from the compiled address of the kernel image to its
	 * actual address in memory.
	 *
	 * This is non-zero if kernel address space layout randomization (KASLR)
	 * is enabled.
	 */
	uint64_t kaslr_offset;
};

/**
 * An ELF file which is mapped into a program.
 *
 * This is parsed from the @c NT_FILE note of a crash dump or
 * <tt>/proc/$pid/maps</tt> of a running program.
 */
struct file_mapping {
	/** Path of the file. */
	char *path;
	/** ELF handle. */
	Elf *elf;
	/** Starting virtual address in the program's address space. */
	uint64_t start;
	/**
	 * One byte after the last virtual address in the program's address
	 * space.
	 */
	uint64_t end;
	/** Starting offset in the file. */
	uint64_t file_offset;
};

struct drgn_cleanup {
	void (*cb)(void *);
	void *arg;
	struct drgn_cleanup *next;
};

struct drgn_program {
	/** @privatesection */
	struct drgn_memory_reader *reader;
	struct drgn_type_index *tindex;
	struct drgn_symbol_index *sindex;
	union {
		struct vmcoreinfo vmcoreinfo;
		struct {
			struct file_mapping *mappings;
			size_t num_mappings;
		};
	};
	struct drgn_cleanup *cleanup;
	enum drgn_program_flags flags;
	bool little_endian;
};

/**
 * Initialize the common part of a @ref drgn_program.
 *
 * This should only be called by @ref drgn_program initializers.
 *
 * @param[in] prog Program to initialize.
 * @param[in] reader Memory reader to use.
 * @param[in] tindex Type index to use.
 * @param[in] sindex Symbol index to use.
 * anything else is deinitialized.
 */
void drgn_program_init(struct drgn_program *prog,
		       struct drgn_memory_reader *reader,
		       struct drgn_type_index *tindex,
		       struct drgn_symbol_index *sindex);

/**
 * Deinitialize a @ref drgn_program.
 *
 * This should only be used if the program was created directly with
 * <tt>drgn_program_init_*</tt>. If the program was created with
 * <tt>drgn_program_from_*</tt>, this shouldn't be used, as it is called by @ref
 * drgn_program_destroy().
 *
 * @param[in] prog Program to deinitialize.
 */
void drgn_program_deinit(struct drgn_program *prog);

/**
 * Add a callback to be called when @ref drgn_program_deinit() is called.
 *
 * The callbacks are called in reverse order of the order they were added in.
 *
 * @param[in] cb Callback.
 * @param[in] arg Argument to callback.
 */
struct drgn_error *drgn_program_add_cleanup(struct drgn_program *prog,
					    void (*cb)(void *), void *arg);

/**
 * Remove a cleanup callback previously added by @ref
 * drgn_program_add_cleanup().
 *
 * This removes the most recently added callback with the given @p cb and @p
 * arg.
 *
 * @return @c true if a callback with the given argument was present, @c false
 * if not.
 */
bool drgn_program_remove_cleanup(struct drgn_program *prog, void (*cb)(void *),
				 void *arg);

/**
 * Implement @ref drgn_program_from_core_dump() on an allocated @ref
 * drgn_program.
 */
struct drgn_error *drgn_program_init_core_dump(struct drgn_program *prog,
					       const char *path, bool verbose);

/**
 * Implement @ref drgn_program_from_kernel() on an allocated @ref drgn_program.
 */
struct drgn_error *drgn_program_init_kernel(struct drgn_program *prog,
					    bool verbose);

/**
 * Implement @ref drgn_program_from_pid() on an allocated @ref drgn_program.
 */
struct drgn_error *drgn_program_init_pid(struct drgn_program *prog, pid_t pid);

/**
 * Initialize a @ref drgn_program from manually-created memory segments, types,
 * and symbols.
 *
 * This is mostly useful for testing.
 *
 * @param[in] prog Program to initialize.
 * @param[in] word_size See @ref drgn_program_word_size().
 * @param[in] little_endian See @ref drgn_program_is_little_endian().
 * @param[in] segments See @ref drgn_mock_memory_reader_create().
 * @param[in] num_segments See @ref drgn_mock_memory_reader_create().
 * @param[in] types See @ref drgn_mock_type_index_create().
 * @param[in] symbols See @ref drgn_mock_symbol_index_create().
 */
struct drgn_error *
drgn_program_init_mock(struct drgn_program *prog, uint8_t word_size,
		       bool little_endian,
		       struct drgn_mock_memory_segment *segments,
		       size_t num_segments, struct drgn_mock_type *types,
		       struct drgn_mock_symbol *symbols);

/** Return the maximum word value for a program. */
static inline uint64_t drgn_program_word_mask(struct drgn_program *prog)
{
	return drgn_program_word_size(prog) == 8 ? UINT64_MAX : UINT32_MAX;
}

/** @} */

#endif /* DRGN_PROGRAM_H */
