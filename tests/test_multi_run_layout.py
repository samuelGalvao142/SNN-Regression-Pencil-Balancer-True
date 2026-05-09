from pathlib import Path
import sys
import unittest
import shutil
import uuid


REPO_ROOT = Path(__file__).resolve().parents[1]
SNN_MAIN = REPO_ROOT / "SNN-regression-main"
if str(SNN_MAIN) not in sys.path:
    sys.path.insert(0, str(SNN_MAIN))

from src.Dataset.multi_run import discover_camera_runs, summarize_camera_runs


class MultiRunLayoutTests(unittest.TestCase):
    def setUp(self):
        self.temp_root = REPO_ROOT / "tests" / f"_tmp_multi_run_layout_{uuid.uuid4().hex}"
        self.temp_root.mkdir(parents=True)

    def tearDown(self):
        if self.temp_root.exists():
            shutil.rmtree(self.temp_root, ignore_errors=True)

    def test_discovers_two_cameras_across_multiple_runs(self):
        dataset_root = self.temp_root

        run1 = dataset_root / "raw" / "run_001"
        run1.mkdir(parents=True)
        (run1 / "run_001_cam1.aedat4").write_bytes(b"")
        (run1 / "run_001_cam1_hough.csv").write_text("timestamp_us,slope,intercept\n1,0.1,10\n", encoding="utf-8")
        (run1 / "run_001_cam2.aedat4").write_bytes(b"")
        (run1 / "run_001_cam2_hough.csv").write_text("timestamp_us,slope,intercept\n1,0.2,20\n", encoding="utf-8")

        run2 = dataset_root / "raw" / "run_002"
        run2.mkdir(parents=True)
        (run2 / "cam1.aedat4").write_bytes(b"")
        (run2 / "cam1_hough.csv").write_text("timestamp_us,slope,intercept\n1,0.3,30\n", encoding="utf-8")
        (run2 / "cam2.aedat4").write_bytes(b"")
        (run2 / "cam2_hough.csv").write_text("timestamp_us,slope,intercept\n1,0.4,40\n", encoding="utf-8")

        records = discover_camera_runs(dataset_root)
        self.assertEqual(len(records), 4)

        summary = summarize_camera_runs(records)
        self.assertEqual(summary[1], ["run_001", "run_002"])
        self.assertEqual(summary[2], ["run_001", "run_002"])


if __name__ == "__main__":
    unittest.main()
