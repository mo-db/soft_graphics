## Basics
- add group sudo
- install build-essential
- install git
- install spice-vdagent (clipboard sharing)

## github ssh tutorial:
- `ssh-keygen -t ed25519 -C "moritz.huber@protonmail.com"`
- `eval "$(ssh-agent -s)"`
- `ssh-add ~/.ssh/id_ed25519`

## Enable ssh service
- start ssh
- enable ssh

## connect with $TERM export
- (optional) `toe -ah` (show terminal types)
- `ssh -t moritz@192.168.64.6 "export TERM=xterm-256color; cd /home/moritz/Repos/pg2/; bash"`

## install neovim
- appimage -> chmod u+x -> into local/bin
- create .bash_aliases -> alias nvim=~/.local/bin/nvim-linux-arm64.appimage
- git clone nvim_config
- (optional) install tmux

# unix commands
