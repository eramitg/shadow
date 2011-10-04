/**
 * The Shadow Simulator
 *
 * Copyright (c) 2010-2011 Rob Jansen <jansen@cs.umn.edu>
 *
 * This file is part of Shadow.
 *
 * Shadow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Shadow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Shadow.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHD_ENGINE_H_
#define SHD_ENGINE_H_

#include <glib.h>

typedef enum _EngineStorage EngineStorage;

enum _EngineStorage {
	NODES, NETWORKS, CDFS,
};

typedef struct _Engine Engine;

struct _Engine {
	/* general configuration options for the simulation */
	Configuration* config;

	/* global simulation time, rough approximate if multi-threaded */
	SimulationTime clock;
	/* minimum allowed time jump when sending events between nodes */
	SimulationTime minTimeJump;
	/* start of current window of execution */
	SimulationTime executeWindowStart;
	/* end of current window of execution (start + min_time_jump) */
	SimulationTime executeWindowEnd;

	/*
	 * Keep track of all sorts of global info: simulation nodes, networks, etc.
	 * all indexed by ID.
	 */
	Registry* registry;

	/* if single threaded, use this global event priority queue. if multi-
	 * threaded, use this for non-node events */
	GAsyncQueue* masterEventQueue;

	/* if multi-threaded, we use a worker pool */
	GThreadPool* workerPool;

	/* holds a thread-private key that each thread references to get a private
	 * instance of a worker object
	 */
	GPrivate* workerKey;

	/*
	 * condition that signals when all node's events have been processed in a
	 * given execution interval.
	 */
	GCond* workersIdle;

	/*
	 * before signaling the engine that the workers are idle, it must be idle
	 * to accept the signal.
	 */
	GMutex* engineIdle;

	/*
	 * these values are modified during simulation and must be protected so
	 * they are thread safe
	 */
	struct {
		/* id counter for worker objects */
		volatile gint workerIDCounter;

		/* id counter for node objects */
		volatile gint nodeIDCounter;

		/* the simulation should attempt to end immediately if this is > 0 */
		volatile gint isKilled;

		/* number of nodes left to process in current interval */
		volatile gint nNodesToProcess;
	} protect;
	MAGIC_DECLARE;
};

Engine* engine_new(Configuration* config);
void engine_free(Engine* engine);
gint engine_run(Engine* engine);
void engine_pushEvent(Engine* engine, Event* event);
gpointer engine_lookup(Engine* engine, EngineStorage type, gint id);

gint engine_generateWorkerID(Engine* engine);
gint engine_generateNodeID(Engine* engine);
gint engine_getNumThreads(Engine* engine);
SimulationTime engine_getMinTimeJump(Engine* engine);
SimulationTime engine_getExecutionBarrier(Engine* engine);
gboolean engine_isKilled(Engine* engine);
void engine_notifyNodeProcessed(Engine* engine);

#endif /* SHD_ENGINE_H_ */
