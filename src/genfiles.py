import sys
from disassemble_sound import main as dmain
from assemble_sound import main as amain
from extract_sequences import main as emain
from fs import open_fs

FS = open_fs('mem://')


def pass1():
    sys.argv = [sys.argv[0], 'baserom.us.z64', '5748512', '97856', '5846368',
                '2216704', 'samples', 'bank']
    dmain(FS)


def pass2():
    sys.argv = [sys.argv[0], 'samples', 'bank', 'sound_data.ctl', 'ctl_header',
                'sound_data.tbl', 'tbl_header', '-DVERSION_US=1', '--endian',
                'little', '--bitwidth', '64']
    amain(FS)


def pass3():
    emain(FS)


def pass4():
    sys.argv = [sys.argv[0], '--sequences', 'sequences.bin', 'seqh.bin', 'bank_sets', 'bank', 'sequences.json', 'sequences/00_sound_player.m64', 'sequences/01_cutscene_collect_star.m64', 'sequences/02_menu_title_screen.m64', 'sequences/03_level_grass.m64', 'sequences/04_level_inside_castle.m64', 'sequences/05_level_water.m64', 'sequences/06_level_hot.m64', 'sequences/07_level_boss_koopa.m64', 'sequences/08_level_snow.m64', 'sequences/09_level_slide.m64', 'sequences/0A_level_spooky.m64', 'sequences/0B_event_piranha_plant.m64', 'sequences/0C_level_underground.m64', 'sequences/0D_menu_star_select.m64', 'sequences/0E_event_powerup.m64', 'sequences/0F_event_metal_cap.m64', 'sequences/10_event_koopa_message.m64', 'sequences/11_level_koopa_road.m64', 'sequences/12_event_high_score.m64', 'sequences/13_event_merry_go_round.m64', 'sequences/14_event_race.m64', 'sequences/15_cutscene_star_spawn.m64', 'sequences/16_event_boss.m64', 'sequences/17_cutscene_collect_key.m64', 'sequences/18_event_endless_stairs.m64', 'sequences/19_level_boss_koopa_final.m64', 'sequences/1A_cutscene_credits.m64', 'sequences/1B_event_solve_puzzle.m64', 'sequences/1C_event_toad_message.m64', 'sequences/1D_event_peach_message.m64', 'sequences/1E_cutscene_intro.m64', 'sequences/1F_cutscene_victory.m64', 'sequences/20_cutscene_ending.m64', 'sequences/21_menu_file_select.m64', 'sequences/22_cutscene_lakitu.m64', '-DVERSION_US=1', '--endian', 'little', '--bitwidth', '64']
    amain(FS)


if __name__ == '__main__':
    pass1()
    pass2()
    pass3()
    pass4()

    for file in ['sound_data.ctl', 'sound_data.tbl', 'bank_sets', 'sequences.bin']:
        print('writing ' + file + '...')
        with FS.open(file, 'rb') as f:
            data = f.read()
        with open(file, 'wb') as f:
            f.write(data)
