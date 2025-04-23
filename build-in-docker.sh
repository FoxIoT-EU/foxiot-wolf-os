#!/bin/sh

# build-in-docker.sh - Build the FoxIoT gateway OS using Docker
# Usage:
#   ./build-in-docker.sh [--rebuild-image] [--clean] [--help] <distro-name>

IMAGE_NAME="wolf-os-builder"
DOCKERFILE_PATH="docker/Dockerfile"
BUILD_DIR="$(pwd)"
WORKDIR_IN_CONTAINER="/project"

REBUILD_IMAGE=0
CLEAN=0
DISTRO=""

print_help() {
  echo "Usage: $0 [OPTIONS] <distro-name>"
  echo
  echo "Build the FoxIoT gateway OS inside Docker."
  echo
  echo "Options:"
  echo "  --rebuild-image     Rebuild the Docker image even if it exists"
  echo "  --clean             Clean previous build artifacts"
  echo "  --list-distros      List available distros in the 'distro/' directory"
  echo "  --help              Show this help message"
  echo
  echo "Arguments:"
  echo "  <distro-name>       Required. Name of the distro to build (e.g., example)"
  echo
  echo "Examples:"
  echo "  $0 example"
  echo "  $0 --clean example"
  echo "  $0 --rebuild-image example"
  echo "  $0 --list-distros"
}

list_distros() {
  echo "Available distros:"
  for d in "$BUILD_DIR"/distro/*; do
    [ -d "$d" ] && echo "  - $(basename "$d")"
  done
}

# --- Parse arguments ---
while [ "$#" -gt 0 ]; do
  case "$1" in
    --rebuild-image)
      REBUILD_IMAGE=1
      ;;
    --clean)
      CLEAN=1
      ;;
    --list-distros)
      list_distros
      exit 0
      ;;
    --help)
      print_help
      exit 0
      ;;
    --*)
      echo "Unknown option: $1"
      echo "Run with --help to see available options."
      exit 1
      ;;
    *)
      DISTRO="$1"
      ;;
  esac
  shift
done

# --- Clean build artifacts if requested ---
if [ "$CLEAN" -eq 1 ]; then
  echo "Cleaning build artifacts..."
  rm -rf build/
  [ -z "$DISTRO" ] && exit 0
fi

# --- Build Docker image if needed ---
if [ "$REBUILD_IMAGE" -eq 1 ] || ! docker image inspect "$IMAGE_NAME" >/dev/null 2>&1; then
  echo "Building Docker image: $IMAGE_NAME"
  docker build -t "$IMAGE_NAME" -f "$DOCKERFILE_PATH" .
  if [ $? -ne 0 ]; then
    echo "Docker image build failed."
    exit 1
  fi
  [ -z "$DISTRO" ] && exit 0
else
  echo "Docker image '$IMAGE_NAME' already exists."
fi

if [ -z "$DISTRO" ]; then
  echo "Error: You must specify a distro name"
  list_distros
  echo
  print_help
  exit 1
fi

# --- Check Docker ---
if ! command -v docker >/dev/null 2>&1; then
  echo "Error: Docker is not installed or not in PATH."
  exit 1
fi

# --- Run Docker container and build the selected distro ---
echo "Running build inside Docker container for distro: $DISTRO"
docker run --rm \
  -e DISTRO="$DISTRO" \
  -u "$(id -u):$(id -g)" \
  -v "$BUILD_DIR":"$WORKDIR_IN_CONTAINER" \
  -w "$WORKDIR_IN_CONTAINER" \
  "$IMAGE_NAME"

EXIT_CODE=$?
if [ "$EXIT_CODE" -eq 0 ]; then
  printf "\033[1;32m✅ Build completed successfully.\033[0m\n"
else
  printf "\033[1;31m❌ Build failed with exit code %d.\033[0m\n" "$EXIT_CODE"
fi

exit "$EXIT_CODE"
