// For capturing the std::out and std::err during test runs
struct TestPrinter {
  public:
    TestPrinter() : streamMutex_() {};

    // If the thread ID is for a known thread, return its
    // existing stream
    std::stringstream* getStreamForThread(std::thread::id id) {
        std::shared_lock r_lock(streamMutex_);
        if (streams.count(id)) {
            return &(streams.at(id));
        }

        return nullptr;
    }

    /* In TestFramework, this is what's called instead of cout. If we haven't
     * started our test runs yet, we can just use cout. If we're running
     * parallel tests, we need to capture the full output for each test in a
     * thread safe way.
     *
     * Store a mapping of thread ID to ostream. Use a shared lock for threads
     * just fetching streams. Use a unique_lock for threads that put new streams
     * in the map values to a stream.
     *
     * The main thread can continue using cout.
     */
    std::ostream& getStream() {
        const auto this_id = std::this_thread::get_id();
        if (startedTestRun_ && this_id != mainThread_) {
            // See if we already have a stream for this thread
            auto* stream = getStreamForThread(this_id);
            if (stream) {
                return *stream;
            }

            // If we don't we need to lock the writer ability on the lock and
            // add a new stream
            std::unique_lock w_lock(streamMutex_);
            streams.emplace(this_id, std::stringstream());
            return streams.at(this_id);
        }

        return std::cout;
    }

  void startTests(std::thread::id id) {
      startedTestRun_ = true;
      mainThread_ = id;
  }

  private:
    std::unordered_map<std::thread::id, std::stringstream> streams;
    std::shared_mutex streamMutex_;
    std::thread::id mainThread_;
    bool startedTestRun_ = false;
};

