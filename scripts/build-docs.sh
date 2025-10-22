#!/bin/bash

# Build and validate JenLib documentation
# Usage: ./scripts/build-docs.sh [clean|validate|serve]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DOCS_DIR="$PROJECT_ROOT/docs"
HTML_DIR="$DOCS_DIR/html"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if Doxygen is installed
check_doxygen() {
    if ! command -v doxygen &> /dev/null; then
        print_error "Doxygen is not installed. Please install it first:"
        echo "  Ubuntu/Debian: sudo apt-get install doxygen graphviz"
        echo "  macOS: brew install doxygen graphviz"
        echo "  Windows: Download from https://www.doxygen.nl/download.html"
        exit 1
    fi

    if ! command -v dot &> /dev/null; then
        print_warning "Graphviz (dot) is not installed. Diagrams will not be generated."
        echo "  Ubuntu/Debian: sudo apt-get install graphviz"
        echo "  macOS: brew install graphviz"
    fi

    print_success "Doxygen is available: $(doxygen --version)"
}

# Function to clean previous builds
clean_docs() {
    print_status "Cleaning previous documentation builds..."

    if [ -d "$HTML_DIR" ]; then
        rm -rf "$HTML_DIR"
        print_success "Removed existing HTML documentation"
    fi

    # Clean any temporary files
    find "$DOCS_DIR" -name "*.tmp" -delete 2>/dev/null || true
}

# Function to build documentation
build_docs() {
    print_status "Building documentation..."

    # Check if Doxyfile exists
    if [ ! -f "$DOCS_DIR/Doxyfile" ]; then
        print_error "Doxyfile not found at $DOCS_DIR/Doxyfile"
        exit 1
    fi

    # Change to project root for relative paths
    cd "$PROJECT_ROOT"

    # Run Doxygen
    if doxygen "$DOCS_DIR/Doxyfile"; then
        print_success "Documentation built successfully"
    else
        print_error "Documentation build failed"
        exit 1
    fi

    # Verify HTML was generated
    if [ ! -d "$HTML_DIR" ]; then
        print_error "HTML documentation was not generated"
        exit 1
    fi

    # Count generated files
    html_count=$(find "$HTML_DIR" -name "*.html" | wc -l)
    print_success "Generated $html_count HTML files"
}

# Function to validate documentation
validate_docs() {
    print_status "Validating documentation structure..."

    if [ ! -d "$HTML_DIR" ]; then
        print_error "HTML documentation not found. Run build first."
        exit 1
    fi

    # Check for main index file
    if [ ! -f "$HTML_DIR/index.html" ]; then
        print_error "Main documentation index.html is missing"
        exit 1
    else
        print_success "Found index.html"
    fi

    print_success "Documentation validation completed"
}

# Function to serve documentation locally
serve_docs() {
    if [ ! -d "$HTML_DIR" ]; then
        print_error "HTML documentation not found. Run build first."
        exit 1
    fi

    print_status "Starting local documentation server..."
    print_status "Documentation will be available at: http://localhost:8000"
    print_status "Press Ctrl+C to stop the server"

    cd "$HTML_DIR"

    # Try different methods to serve the files
    if command -v python3 &> /dev/null; then
        python3 -m http.server 8000
    elif command -v python &> /dev/null; then
        python -m SimpleHTTPServer 8000
    elif command -v php &> /dev/null; then
        php -S localhost:8000
    else
        print_error "No suitable HTTP server found. Please install Python, PHP, or use a different method."
        exit 1
    fi
}

# Function to show help
show_help() {
    echo "JenLib Documentation Builder"
    echo ""
    echo "Usage: $0 [COMMAND]"
    echo ""
    echo "Commands:"
    echo "  build     Build the documentation (default)"
    echo "  clean     Clean previous builds"
    echo "  validate  Validate documentation structure"
    echo "  serve     Serve documentation locally on http://localhost:8000"
    echo "  all       Clean, build, and validate"
    echo "  help      Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                # Build documentation"
    echo "  $0 clean          # Clean previous builds"
    echo "  $0 all            # Clean, build, and validate"
    echo "  $0 serve          # Serve documentation locally"
}

# Main script logic
main() {
    local command="${1:-build}"

    case "$command" in
        "clean")
            check_doxygen
            clean_docs
            ;;
        "build")
            check_doxygen
            build_docs
            ;;
        "validate")
            validate_docs
            ;;
        "serve")
            serve_docs
            ;;
        "all")
            check_doxygen
            clean_docs
            build_docs
            validate_docs
            print_success "Documentation build completed successfully!"
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        *)
            print_error "Unknown command: $command"
            show_help
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"
