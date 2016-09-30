#include <application/event.h>
#include <application/service.h>
#include <application/application.h>

struct ExecutedService
{
	bool executed;
	~ExecutedService() {}
	ExecutedService() 
	{
		executed = false;
		
		marbles::shared_service active = marbles::service::active();
		active->post([this]() { this->Execute(); });
	}

	void Execute() 
	{
		executed = true; 
		marbles::service::active()->stop();
	}
};

struct ApplicationStop
{
	typedef marbles::event<> UpdateEvent;
	typedef UpdateEvent::shared_handler UpdateHandler;
	int count;
	int stop;
	int label;
	//UpdateEvent update;
	//UpdateHandler updateHandler;

	ApplicationStop(const int id, const int end) 
	: count(0)
	, stop(end)
	, label(id)
	{
		//updateHandler = update += [this]() { this->OnUpdate(); };
		//update();
        marbles::shared_service active = marbles::service::active();
        active->post([this]() { this->OnUpdate(); });
    }

	~ApplicationStop() 
	{
		//update -= updateHandler;
		//update.clear();
	}

	void OnUpdate() 
	{
		++count;
		if (count != stop)
		{
			//update();
            marbles::shared_service active = marbles::service::active();
            active->post([this]() { this->OnUpdate(); });
        }
		else
		{	// We have a winner stop the application
			marbles::application::get()->stop(0);
		}
	}
};

TEST(service, single_thread_test)
{
	const int numCyclesToStop = 1000;
	bool randomTask = false;
	int label = 0;
	marbles::application app;
	marbles::shared_service service = app.start<ExecutedService>();
	marbles::vector<marbles::shared_service> racers;
	racers.push_back(app.start<ApplicationStop>(label++, numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(label++, numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(label++, numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(label++, numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(label++, numCyclesToStop));

	app.post([&randomTask]() { randomTask = true; });
	app.run(1);

	EXPECT_EQ(true, randomTask);
	EXPECT_EQ(true, service->provider<ExecutedService>()->executed);
	marbles::shared_service winner = *racers.begin();
	for(auto i : racers)
	{
		if (winner->provider<ApplicationStop>()->count < i->provider<ApplicationStop>()->count)
		{
			winner = i;
		}
	}
	const int stopCount = winner->provider<ApplicationStop>()->count;
	EXPECT_EQ(numCyclesToStop, stopCount);
}

TEST(service, multiple_thread_test)
{
	const int numCyclesToStop = 1000;
	bool randomTask = false;
	marbles::application app;
	marbles::shared_service service = app.start<ExecutedService>();
	marbles::vector<marbles::shared_service> racers;
	racers.push_back(app.start<ApplicationStop>(0, numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(1, numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(2, numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(3, numCyclesToStop));
	racers.push_back(app.start<ApplicationStop>(4, numCyclesToStop));

	app.post([&randomTask]() { randomTask = true; });
	app.run();

	EXPECT_EQ(true, randomTask);
	EXPECT_EQ(true, service->provider<ExecutedService>()->executed);
	marbles::shared_service winner = *racers.begin();
	for(auto i : racers)
	{
		if (winner->provider<ApplicationStop>()->count < i->provider<ApplicationStop>()->count)
		{
			winner = i;
		}
	}
	const int stopCount = winner->provider<ApplicationStop>()->count;
	EXPECT_EQ(numCyclesToStop, stopCount);
}
