import unittest
from src.main import run

class TestApp(unittest.TestCase):
    def test_run(self):
        self.assertTrue(callable(run))

if __name__ == '__main__':
    unittest.main()
