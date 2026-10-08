# Build and Deployment Rule

Whenever modifying code (C++, CMake, QML, or assets):
1. **Always build automatically:** Immediately run `build.bat` after making code changes.
2. **Deploy automatically when needed:** Run `deploy.bat` if new Qt components or dependencies are added or updated.
3. **Handle errors proactively:** Fix any compiler or linker errors autonomously without requiring the user to compile manually.
4. **Never ask the user to build:** The user expects all builds, compilation steps, and execution verifications to be fully handled by the agent.
