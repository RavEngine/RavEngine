module.exports = {
    "dataSource": "milestones",
    "prefix": "",
    "groupBy": "label",
    "milestoneMatch": "{{tag_name}}",
    "changelogFilename": "CHANGELOG.md",

    "template": {
      "release": function (placeholders) {
	var date = placeholders.date.replace(/(..)\/(..)\/(....)/, "$3-$2-$1");
	return `## ${placeholders.release} (${date})\n${placeholders.body}`
      },
    }
}
