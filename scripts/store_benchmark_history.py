#!/usr/bin/env python3
"""
Store benchmark results in historical database.

This script appends benchmark results to a time-series database stored
in a separate git branch (benchmark-data) for long-term trend analysis.
"""

import argparse
import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Any, List


def get_git_info() -> Dict[str, str]:
    """Get current git commit and branch information."""
    try:
        commit = subprocess.check_output(
            ["git", "rev-parse", "HEAD"], text=True
        ).strip()
        commit_short = subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"], text=True
        ).strip()
        branch = subprocess.check_output(
            ["git", "rev-parse", "--abbrev-ref", "HEAD"], text=True
        ).strip()
        return {
            "commit": commit,
            "commit_short": commit_short,
            "branch": branch,
        }
    except subprocess.CalledProcessError:
        return {
            "commit": "unknown",
            "commit_short": "unknown",
            "branch": "unknown",
        }


def load_benchmark_results(json_file: Path) -> List[Dict[str, Any]]:
    """Load benchmark results from JSON file."""
    with open(json_file) as f:
        return json.load(f)


def append_to_history(
    results: List[Dict[str, Any]],
    history_file: Path,
    git_info: Dict[str, str],
    run_id: str,
) -> None:
    """Append benchmark results to historical database."""
    # Load existing history if it exists
    if history_file.exists():
        with open(history_file) as f:
            history = json.load(f)
    else:
        history = {
            "schema_version": "1.0",
            "description": "Historical benchmark results for fastcond",
            "runs": [],
        }

    # Create new run entry
    run_entry = {
        "run_id": run_id,
        "timestamp": datetime.now(timezone.utc).isoformat(),
        "git": git_info,
        "results": results,
    }

    # Append to history
    history["runs"].append(run_entry)

    # Write back
    with open(history_file, "w") as f:
        json.dump(history, f, indent=2)

    print(f"✅ Appended {len(results)} results to {history_file}", file=sys.stderr)
    print(f"   Total historical runs: {len(history['runs'])}", file=sys.stderr)


def setup_benchmark_data_branch(repo_path: Path) -> bool:
    """
    Setup or switch to benchmark-data branch.
    Returns True if successful, False otherwise.
    """
    try:
        # Check if branch exists locally or remotely
        result = subprocess.run(
            ["git", "show-ref", "--verify", "refs/heads/benchmark-data"],
            cwd=repo_path,
            capture_output=True,
        )
        branch_exists = result.returncode == 0

        if not branch_exists:
            # Check if it exists remotely
            result = subprocess.run(
                ["git", "ls-remote", "--heads", "origin", "benchmark-data"],
                cwd=repo_path,
                capture_output=True,
                text=True,
            )
            if result.stdout.strip():
                # Exists remotely, fetch it
                subprocess.run(
                    ["git", "fetch", "origin", "benchmark-data:benchmark-data"],
                    cwd=repo_path,
                    check=True,
                )
                print("📥 Fetched benchmark-data branch from remote", file=sys.stderr)
            else:
                # Create new orphan branch
                print("🆕 Creating new benchmark-data branch", file=sys.stderr)
                subprocess.run(
                    ["git", "checkout", "--orphan", "benchmark-data"],
                    cwd=repo_path,
                    check=True,
                )
                subprocess.run(["git", "rm", "-rf", "."], cwd=repo_path, check=True)

                # Create initial README
                readme_content = """# Benchmark Data

This branch stores historical benchmark results for trend analysis.

Files:
- `benchmark-history.json` - Main benchmark results history
- `platform-history.json` - Cross-platform benchmark history

Do not manually edit these files - they are managed by CI automation.
"""
                readme_path = repo_path / "README.md"
                with open(readme_path, "w") as f:
                    f.write(readme_content)

                subprocess.run(["git", "add", "README.md"], cwd=repo_path, check=True)
                subprocess.run(
                    [
                        "git",
                        "commit",
                        "-m",
                        "Initialize benchmark-data branch for historical storage",
                    ],
                    cwd=repo_path,
                    check=True,
                )
                print("✅ Created benchmark-data branch", file=sys.stderr)
                return True
        else:
            # Switch to existing branch
            subprocess.run(
                ["git", "checkout", "benchmark-data"], cwd=repo_path, check=True
            )
            print("✅ Switched to benchmark-data branch", file=sys.stderr)

        return True

    except subprocess.CalledProcessError as e:
        print(f"❌ Failed to setup benchmark-data branch: {e}", file=sys.stderr)
        return False


def commit_and_push_history(repo_path: Path, message: str) -> bool:
    """Commit and push changes to benchmark-data branch."""
    try:
        # Add only files that exist
        files_to_add = []
        if (repo_path / "benchmark-history.json").exists():
            files_to_add.append("benchmark-history.json")
        if (repo_path / "platform-history.json").exists():
            files_to_add.append("platform-history.json")

        if not files_to_add:
            print("ℹ️  No history files to commit", file=sys.stderr)
            return True

        subprocess.run(
            ["git", "add"] + files_to_add,
            cwd=repo_path,
            check=True,
        )

        # Check if there are changes to commit
        result = subprocess.run(
            ["git", "diff", "--cached", "--quiet"], cwd=repo_path, capture_output=True
        )

        if result.returncode != 0:  # There are changes
            subprocess.run(["git", "commit", "-m", message], cwd=repo_path, check=True)
            subprocess.run(
                ["git", "push", "origin", "benchmark-data"], cwd=repo_path, check=True
            )
            print("✅ Committed and pushed to benchmark-data branch", file=sys.stderr)
        else:
            print("ℹ️  No changes to commit", file=sys.stderr)

        return True

    except subprocess.CalledProcessError as e:
        print(f"❌ Failed to commit/push: {e}", file=sys.stderr)
        return False


def main():
    parser = argparse.ArgumentParser(
        description="Store benchmark results in historical database"
    )
    parser.add_argument(
        "results_file", type=Path, help="Benchmark results JSON file to store"
    )
    parser.add_argument(
        "--history-type",
        choices=["benchmark", "platform"],
        default="benchmark",
        help="Type of history to store (default: benchmark)",
    )
    parser.add_argument(
        "--run-id",
        help="Unique run identifier (default: auto-generated from timestamp)",
    )
    parser.add_argument(
        "--no-push",
        action="store_true",
        help="Don't push to remote (useful for local testing)",
    )

    args = parser.parse_args()

    if not args.results_file.exists():
        print(f"❌ Results file not found: {args.results_file}", file=sys.stderr)
        return 1

    # Generate run ID if not provided
    run_id = args.run_id or datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S")

    # Get git info
    git_info = get_git_info()
    print(
        f"📊 Storing results from commit {git_info['commit_short']} ({git_info['branch']})",
        file=sys.stderr,
    )

    # Load benchmark results
    results = load_benchmark_results(args.results_file)
    print(f"📥 Loaded {len(results)} benchmark results", file=sys.stderr)

    # Get repository path
    repo_path = Path(__file__).parent.parent

    try:
        # Setup benchmark-data branch
        if not setup_benchmark_data_branch(repo_path):
            return 1

        # Determine history file based on type
        history_filename = (
            "benchmark-history.json"
            if args.history_type == "benchmark"
            else "platform-history.json"
        )
        history_file = repo_path / history_filename

        # Append to history
        append_to_history(results, history_file, git_info, run_id)

        # Commit and push
        if not args.no_push:
            commit_message = (
                f"Add {args.history_type} results: {run_id} "
                f"({git_info['commit_short']})"
            )
            if not commit_and_push_history(repo_path, commit_message):
                return 1

        print(
            f"\n✅ Successfully stored {args.history_type} results in historical database",
            file=sys.stderr,
        )
        return 0

    finally:
        # Always return to original branch and directory
        try:
            subprocess.run(["git", "checkout", "-"], cwd=repo_path, capture_output=True)
        except Exception:
            pass


if __name__ == "__main__":
    sys.exit(main())
