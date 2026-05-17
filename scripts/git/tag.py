import subprocess

def run(cmd):
    return subprocess.run(cmd, text=True)

def main():
    tag = input("Tag name (e.g. 0.1.2.1v-CSC (Please check docs/Version/README.md)): ").strip()

    if not tag:
        print("No tag provided.")
        return

    confirm = input(f"Push tag '{tag}' to remote? (Y/n): ").strip().lower()

    if confirm not in ["y", "yes", ""]:
        print("Cancelled.")
        return

    print("Creating tag...")
    result = run(["git", "tag", tag])

    if result.returncode != 0:
        print("Failed to create tag.")
        return

    print("Pushing tag...")
    result = run(["git", "push", "origin", tag])

    if result.returncode == 0:
        print("Tag pushed successfully.")
    else:
        print("Failed to push tag.")

if __name__ == "__main__":
    main()
    