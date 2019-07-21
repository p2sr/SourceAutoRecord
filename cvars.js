const fs = require('fs');

class Cvar {
    constructor(name, value, games, description) {
        this.name = name;
        this.value = value;
        games.pop();
        this.games = games;
        this.description = description;
    }
};

fs.readFile(process.argv[2] + '/sar.cvars', 'utf-8', (err, data) => {
    if (err) {
        return console.error(err);
    }

    data = '[end_of_cvar]' + data;

    let cvars = [];
    for (let cvar of data.split('[end_of_cvar]')) {
        let data = cvar.split('[cvar_data]');
        if (data.length == 4) {
            cvars.push(new Cvar(data[0], data[1], data[2].split('\n'), data[3]));
        }
    }

    const compareCvar = (a, b) => {
        if (a.startsWith('-') || a.startsWith('+')) {
            a = a.substring(1);
        }
        if (b.startsWith('-') || b.startsWith('+')) {
            b = b.substring(1);
        }
        return a.localeCompare(b);
    };

    let body = '';
    for (let cvar of cvars.sort((a, b) => compareCvar(a.name, b.name))) {
        body += '\n|';
        body += (cvar.games.length > 0) ? `<i title="${cvar.games.join('&#10;')}">${cvar.name}</i>` : cvar.name;
        body += `|${cvar.value}|${cvar.description.replace(/</g, '\\<').replace(/\n/g, '<br>')}|`;
    }

    fs.writeFileSync('doc/cvars.md',
`# SAR: Cvars

|Name|Default|Description|
|---|---|---|${body}`);
});
