# Planner - Schedule Management System

A robust C++ server-based scheduling system with algorithm flexibility and comprehensive messaging architecture.

## 📚 Documentation

### Architecture Documentation
- **[Complete API Documentation](https://ddf172.github.io/Planner/)** - Full Doxygen documentation with UML diagrams
- **[Class Hierarchy](https://ddf172.github.io/Planner/hierarchy.html)** - Visual class inheritance tree
- **[Algorithm Architecture](./server/workspace/markdowns/ALGORITHM_ARCHITECTURE.md)** - Detailed algorithm module design
- **[Communication Protocol](./server/workspace/markdowns/Communictaion.md)** - Message handling specification

### Key Classes & Diagrams
- **[System](https://ddf172.github.io/Planner/classSystem.html)** - Central component container
- **[AlgorithmRunner](https://ddf172.github.io/Planner/classAlgorithmRunner.html)** - Algorithm execution engine
- **[MessageProcessor](https://ddf172.github.io/Planner/classMessageProcessor.html)** - Message handling system
- **[HandlerDispatcher](https://ddf172.github.io/Planner/classHandlerDispatcher.html)** - Message routing

## 🚀 Quick Start

### Build & Run
```bash
cd server/workspace
mkdir build && cd build
cmake ..
make
./server
```

### Generate Documentation
```bash
cd server/workspace
doxygen Doxyfile
# Output in doxy/html/
```
📖 **[Browse Full Documentation](https://ddf172.github.io/Planner/)** | 🏛️ **[View Class Diagrams](https://ddf172.github.io/Planner/inherits.html)**
