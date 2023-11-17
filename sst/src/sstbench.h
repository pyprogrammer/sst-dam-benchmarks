// Copyright 2009-2023 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2023, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _SIMPLE_EXTERNAL_ELEMENT_H
#define _SIMPLE_EXTERNAL_ELEMENT_H

#include <sst/core/component.h>
#include <sst/core/eli/elementinfo.h>
#include <sst/core/link.h>

#include <deque>

#include "../../common/workloads.h"

#define EVENT_PAYLOAD_BYTES 32

class SimData : public SST::Event
{
public:
	SimData() : SST::Event() {}
	void serialize_order(SST::Core::Serialization::serializer &ser) override
	{
		Event::serialize_order(ser);
		ser &data;
	}

	uint64_t data;

	ImplementSerializable(SimData);
};

class Sender
{
public:
	void send(SST::Event *event)
	{
		link->send(event);
		send_receive_delta += 1;
	}
	void recv_handler(SST::Event *event)
	{
		delete event;
		send_receive_delta -= 1;
	}

	bool is_full()
	{
		return send_receive_delta == capacity;
	}

	SST::Link *link;
	uint64_t capacity;

private:
	uint64_t send_receive_delta = 0;
};

class SumElements : public SST::Component
{

public:
	SumElements(SST::ComponentId_t id, SST::Params &params);
	~SumElements();

	void setup();
	void finish();

	void response_handler(SST::Event *event)
	{
		bool was_backpressured = output_link.is_full() && !a_buffer.empty() && !b_buffer.empty();
		if (was_backpressured)
		{
			push_next_output();
			delete event;
		}
		else
		{
			output_link.recv_handler(event);
		}
	}

	void inputAhandler(SST::Event *event)
	{
		if (auto evt = dynamic_cast<SimData *>(event))
		{
			a_buffer.push_back(evt->data);
			if (!(output_link.is_full() || b_buffer.empty()))
			{
				push_next_output();
			}
		}
		delete event;
	}
	void inputBHandler(SST::Event *event)
	{
		if (auto evt = dynamic_cast<SimData *>(event))
		{
			b_buffer.push_back(evt->data);
			if (!(output_link.is_full() || a_buffer.empty()))
			{
				push_next_output();
			}
		}
		delete event;
	}

	SST_ELI_REGISTER_COMPONENT(
		SumElements,
		"mergeElements",
		"SumElements",
		SST_ELI_ELEMENT_VERSION(1, 0, 0),
		"Demonstration of an External Element for SST",
		COMPONENT_CATEGORY_PROCESSOR)

	SST_ELI_DOCUMENT_PORTS(
		{"inputA", "Link to another component", {"mergeElements.SimData", ""}},
		{"inputB", "", {"mergeElements.SimData", ""}},
		{"output_link", "", {"mergeElements.SimData", ""}})

private:
	void push_next_output()
	{
		auto a = a_buffer.front();
		auto b = b_buffer.front();
		a_buffer.pop_front();
		b_buffer.pop_front();

		auto res = a + b;

		auto evt = new SimData;
		evt->data = res + compute_fibonacci(this->fib_repeats);

		output_link.send(evt);
		input_a->send(new SimData);
		input_b->send(new SimData);


		this->repeats++;
		if (this->repeats == this->maxRepeats) {
			primaryComponentOKToEndSim();
		}
	}

	SST::Output output;
	SST::Cycle_t maxRepeats;
	SST::Cycle_t repeats;

	uint64_t fib_repeats;

	SST::Link *input_a;
	SST::Link *input_b;

	std::deque<int32_t> a_buffer;
	std::deque<int32_t> b_buffer;

	Sender output_link;
};

class SignalGenerator : public SST::Component
{
public:
	SignalGenerator(SST::ComponentId_t id, SST::Params &params);
	~SignalGenerator();

	void setup();
	void finish();

	bool clockTick(SST::Cycle_t currentCycle);

	void response_handler(SST::Event *evt)
	{
		output_link.recv_handler(evt);
	}

	SST_ELI_REGISTER_COMPONENT(
		SignalGenerator,
		"mergeElements",
		"SignalGenerator",
		SST_ELI_ELEMENT_VERSION(1, 0, 0),
		"SignalGenerator",
		COMPONENT_CATEGORY_PROCESSOR)

	SST_ELI_DOCUMENT_PORTS(
		{"output_link", "Link to another component", {"mergeElements.SimData", ""}})

private:
	SST::Output output;
	SST::Cycle_t maxRepeats;
	SST::Cycle_t repeats;

	int32_t iter = 0;

	SST_ELI_DOCUMENT_PARAMS(
		{"repeats", "Number of repetitions to make", "10"},
		{"depth", "Channel Depth", "10"})

	Sender output_link;
};

class Checker : public SST::Component
{
public:
	Checker(SST::ComponentId_t id, SST::Params &params);
	~Checker();

	void setup();
	void finish();

	void handleEvent(SST::Event *event)
	{
		if (auto evt = dynamic_cast<SimData *>(event))
		{
			this->repeats++;
			// output.verbose(CALL_INFO, 1, 0, "Checker received: %d.\n", evt->data);
			if (this->repeats == this->maxRepeats)
			{
				primaryComponentOKToEndSim();
			}
		}
		input_link->send(event);
	}

	SST_ELI_REGISTER_COMPONENT(
		Checker,
		"mergeElements",
		"Checker",
		SST_ELI_ELEMENT_VERSION(1, 0, 0),
		"Checker",
		COMPONENT_CATEGORY_PROCESSOR)

	SST_ELI_DOCUMENT_PORTS(
		{"input_link", "Link to another component", {"mergeElements.SimData", ""}})

private:
	SST::Output output;
	SST::Link *input_link;
	uint64_t repeats;
	uint64_t maxRepeats;
};


#endif
