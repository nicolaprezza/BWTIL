rm data/plain/*.dbh
rm data/plain/*.sfm

git add algorithms
git add common
git add data
git add data_structures
git add extern
git add tools
git add git-*
git add Makefile
git add README

git commit -m "$1"

git push origin master
