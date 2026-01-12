import os

base_path = os.getcwd()
filename = 'Scrite-{{VERSION}}.dmg'
volume_name = 'Scrite-{{VERSION}}'
format = 'ULFO'
filesystem = 'APFS'
background = os.path.join(base_path, 'background.png')
window_rect = ((272, 136), (896, 660))
show_status_bar = False
show_tab_view = False
show_toolbar = False
show_pathbar = False
show_sidebar = False
icon_size = 128
icon_locations = {
    'Scrite.app': (256, 300),
    'Applications': (620, 300)
}
files = [ os.path.join(base_path, 'Scrite-{{VERSION}}/Scrite.app') ]
symlinks = { 'Applications': '/Applications' }
