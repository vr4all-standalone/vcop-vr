import json, unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class IdentityTests(unittest.TestCase):
    def test_native_game_identity_and_revision(self):
        path=ROOT/'engine-lock.json'
        self.assertTrue(path.is_file(), 'Independent VC1 engine lock is missing')
        lock=json.loads(path.read_text())
        self.assertEqual(lock['subtarget'],'vcop')
        self.assertEqual(lock['commit'],'f34f02505e32c1993c6a782b6814232cbfc74e36')
    def test_manifest_is_original_model2(self):
        path=ROOT/'game-set.json'
        self.assertTrue(path.is_file(), 'Verified VC1 ROM manifest is missing')
        game=json.loads(path.read_text())
        self.assertEqual(game['set'],'vcop')
        self.assertEqual(game['hardware'],'Model 2')
        self.assertTrue(game['files'])
        self.assertFalse(any('185' in item['name'] for item in game['files']), 'VC2 media retained')
if __name__=='__main__':unittest.main()
