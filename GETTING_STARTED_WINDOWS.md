# Finite Fusion: Windows quick start

This repository is a fork of [RHH's pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion). Development on Windows is done inside **Ubuntu on WSL2** because it is the supported and fastest Windows workflow.

## Milestone 1

The first milestone is deliberately small:

1. clone the repository;
2. compile `pokeemerald.gba`;
3. open it in mGBA;
4. make one visible text change and rebuild it.

No original ROM file is required to compile this decompilation project. Never commit generated ROMs: `*.gba` is already ignored.

## 1. Install WSL2

Open **PowerShell as Administrator**:

```powershell
wsl --install -d Ubuntu
```

Restart Windows if asked. Launch **Ubuntu** from the Start menu and create the Linux username and password it requests.

Confirm that Ubuntu is using WSL2 from PowerShell:

```powershell
wsl --list --verbose
```

If Ubuntu shows version 1, switch it:

```powershell
wsl --set-version Ubuntu 2
```

## 2. Install build dependencies

Run these commands in the **Ubuntu** terminal:

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install -y build-essential binutils-arm-none-eabi gcc-arm-none-eabi libnewlib-arm-none-eabi git libpng-dev python3
```

## 3. Clone Finite Fusion

Keep the project in WSL's Linux filesystem for fast builds:

```bash
mkdir -p ~/projects
cd ~/projects
git clone https://github.com/aracanelli/finite_fusion.git
cd finite_fusion
git remote add upstream https://github.com/rh-hideout/pokeemerald-expansion.git
```

Check both remotes:

```bash
git remote -v
```

`origin` should be `aracanelli/finite_fusion`; `upstream` should be the RHH project.

## 4. Verify and build

From the repository directory:

```bash
bash scripts/check-setup.sh
make -j"$(nproc)"
```

A successful build creates `pokeemerald.gba` in the repository root. Warnings about an RWX load segment can be normal; an `error:` or a nonzero final exit is not.

## 5. Install the development apps

Install these Windows applications:

- [Visual Studio Code](https://code.visualstudio.com/) with Microsoft's **WSL** extension.
- [mGBA](https://mgba.io/downloads.html) for running and debugging the ROM.
- [Porymap](https://github.com/huderlem/porymap/releases) for maps, warps, NPC placement, and encounters. We will configure it after the first successful build.

In the Ubuntu terminal, open the project in VS Code:

```bash
cd ~/projects/finite_fusion
code .
```

If `code` is not found, open VS Code in Windows, install the WSL extension, press `Ctrl+Shift+P`, and run **WSL: Connect to WSL** before retrying.

To browse the current WSL directory in Windows Explorer:

```bash
explorer.exe .
```

You can open `pokeemerald.gba` from that Explorer window with mGBA.

## 6. Daily workflow

Create a branch for each change:

```bash
git switch master
git pull --ff-only origin master
git switch -c feature/my-change
make -j"$(nproc)"
```

After testing:

```bash
git status
git add <files-you-changed>
git commit -m "Describe the change"
git push -u origin feature/my-change
```

Do not add `pokeemerald.gba`, build output, save files, or an original commercial ROM.

## Updating from RHH

Do this intentionally, on its own branch, so expansion updates do not get mixed into game changes:

```bash
git fetch upstream
git switch master
git switch -c maintenance/update-expansion
git merge upstream/master
```

Build and test before merging that branch into `master`. Expansion updates can produce conflicts once the game has substantial custom code.

## Troubleshooting

- **Builds are extremely slow:** ensure the checkout is under `~/projects`, not `/mnt/c/`.
- **A package is missing:** rerun the dependency command in step 2.
- **`arm-none-eabi-gcc` is not found:** run `bash scripts/check-setup.sh` and reinstall the ARM packages.
- **mGBA cannot find the output:** the expected path inside WSL is `~/projects/finite_fusion/pokeemerald.gba`.
- **GitHub rejects a push:** confirm `git remote -v`, then authenticate using Git Credential Manager or GitHub's documented authentication flow.

The upstream reference remains [INSTALL.md](INSTALL.md). This guide only narrows it to our chosen Windows setup.
